#!/usr/bin/env python

# Copyright 2012, Pisellino & Aurelio Pam Pam ;-)

import time
import sys
import argparse
import usb.core
import keypair
import teledoc


sys.path.append(keypair.distroRoot + '/ipc/python')
sys.path.append(keypair.distroRoot + '/interfaces/skype/python')

import Skype

accountName = "teledoc2000"
accountPsw = "teledoc2000"


class ExitException(Exception):
    def __init__(self, msg):
        self.message = msg

    pass


class NoExitArgumentParser(argparse.ArgumentParser):

    def _print_message(self, message, file=None):
        raise ExitException(message)

    def exit(self, status=0, message=None):
        raise ExitException(message)
       
    pass


class LauncherController(object):

    def __init__(self):
        pass
        # self.dev = usb.core.find(idVendor=0x2123, idProduct=0x1010)
        # if self.dev is None:
        #     raise ValueError('Launcher not connected!')
         
        # if self.dev.is_kernel_driver_active(0) is True:
        #     self.dev.detach_kernel_driver(0)

        # self.dev.set_configuration()

    def turretUp(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00]) 

    def turretDown(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00])

    def turretLeft(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00])

    def turretRight(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x08,0x00,0x00,0x00,0x00,0x00,0x00])

    def turretStop(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x20,0x00,0x00,0x00,0x00,0x00,0x00])

    def turretFire(self):
        self.dev.ctrl_transfer(0x21,0x09,0,0,[0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00])

    pass


class CommandParser(object):
    
    # Command tokens
    (UP, DOWN, LEFT, RIGHT, FIRE, HELP, UNKNOWN, TRACK, UNTRACK) = range(0,9)

    def __init__(self):
        self.parser = NoExitArgumentParser('')
        subparsers = self.parser.add_subparsers(dest="subparser", title="Commands", help='')

        # create the parser for the "up" command
        parser_up = subparsers.add_parser('up', help='Moves the launcher up')
        parser_up.add_argument('times', type=int, nargs='?', default=1, help='The number of repetitions')

        # create the parser for the "down" command
        parser_down = subparsers.add_parser('down', help='Moves the launcher down')
        parser_down.add_argument('times', type=int, nargs='?', default=1, help='The number of repetitions')

        # create the parser for the "left" command
        parser_left = subparsers.add_parser('left', help='Moves the launcher left')
        parser_left.add_argument('times', type=int, nargs='?', default=1, help='The number of repetitions')

        # create the parser for the "right" command
        parser_right = subparsers.add_parser('right', help='Moves the launcher right')
        parser_right.add_argument('times', type=int, nargs='?', default=1, help='The number of repetitions')

        # create the parser for the "fire" command
        parser_fire = subparsers.add_parser('fire', help='Shoots the launcher')
        parser_fire.add_argument('times', type=int, nargs='?', default=1, help='The number of repetitions')

        # create the parser for the "track" command
        parser_fire = subparsers.add_parser('track', help='Tracks a given color')
        parser_fire.add_argument('color', type=int, nargs='?', default=30, help='The hue of the color (default: yellow)')

        # create the parser for the "untrack" command
        parser_fire = subparsers.add_parser('untrack', help='Disables tracking')

        # create the parser for the "help" command
        parser_fire = subparsers.add_parser('help', help='Gets this help message')
        return


    def parse(self, cmd):
        # Clean a bit the string
        cmd = cmd.lower()
        cmd = cmd.strip()

        cmds = cmd.split()
        try:
            args = self.parser.parse_args(cmds)

            if args.subparser == "help":
                return (self.HELP, [self.parser.format_help()])
            elif args.subparser == "up":
                return (self.UP, [args.times])
            elif args.subparser == "down":
                return (self.DOWN, [args.times])
            elif args.subparser == "left":
                return (self.LEFT, [args.times])
            elif args.subparser == "right":
                return (self.RIGHT, [args.times])
            elif args.subparser == "fire":
                return (self.FIRE, [args.times])
            elif args.subparser == "track":
                return (self.TRACK, [args.color])
            elif args.subparser == "untrack":
                return (self.UNTRACK, [])
            else:
                return (self.UNKNOWN, [])

        except ExitException as e:
            return (self.HELP, [e.message])

    pass



class SkypeServer():
    
    def __init__(self):
        # Create an instance of the Skype class.
        self.skype = Skype.GetSkype(keypair.keyFileName)
        self.parser = CommandParser()
        self.controller = LauncherController()
        self.tracker = None

        self.loggedIn = False
        self.devices = None

        # Callback initialization
        Skype.Account.OnPropertyChange = self.onChangeAccount
        self.skype.OnAvailableDeviceListChange = self.onAvailableDeviceListChange
        self.skype.OnMessage = self.onMessageStatus
        return

    def onChangeAccount(self, account, property_name):
	if property_name == 'status':
            if account.status == 'LOGGED_IN':
                loggedIn = True
                print('Login complete.')


    def onAvailableDeviceListChange(self, skype):
        self.devices = skype.GetAvailableVideoDevices()
        if self.devices is None or len(self.devices[0]) == 0:
                print "No video devices found"
                self.devices = None
               

    def initializeTracking(self, color):
        # TODO: adjust these constants
        self.tracker = teledoc.TeledocRenderer(50, color, 5)

        while self.devices == None:
            self.onAvailableDeviceListChange(self.skype)
            time.sleep(1)

        # Get the first device
        video = self.skype.GetPreviewVideo('MEDIA_VIDEO', self.devices[0][0], self.devices[1][0])
        video.SetRemoteRendererId(self.tracker.getKey())


    def deinitializeTracking(self):
        video.SetRemoteRendererId(0)
        self.tracker = None
        pass

    def login(self):
        print accountName
        account = self.skype.GetAccount(accountName)
        account.LoginWithPassword(accountPsw, False, False)
        while self.loggedIn == False:
            time.sleep(1)
        

    def start(self):

        self.login()

        # Main loop
        try:
            while True:
                self.schedule_activities()
        except KeyboardInterrupt:
            print "\nExiting TeleDOC MissileLauncher\n"
        return

    def schedule_activities():
        """ Here we can insert code that needs to be run 
            periodically on the system """
        if self.tracker is not None:
            if self.tracker.newFrameAvailable():
                print self.tracker.getCurrentPosition()
            else:
                timer.sleep(0.05)
        else:
            timer.sleep(0.2)

    def onCallStatus(self, call, status):
        # TODO: update to skypekit
        if status == Skype4Py.clsRinging:
            print "Accepting call from %s" % call.PartnerHandle
            call.Answer()
            time.sleep(2)
            call.StartVideoSend()


    def onMessageStatus(self, message, changesInboxTimestamp, supersedesHistoryMessage, conversation):
        # Dispatch all messages
        if message.author != accountName:
            # I received a message
            res = self.doCommand(message.body_xml)
            conversation.PostText(res, False)


    def doCommand(self, cmd):
        (cmdId, args) = self.parser.parse(cmd)
        
        # Handle error messages
        if cmdId == CommandParser.UNKNOWN:
            return "Unknown command. Use 'help' for usage description"
        elif cmdId == CommandParser.HELP:
            return args[0]

        # Times to repeat
        times = 1
        if len(args) > 0:
            times = args[0]

        # Real commands
        if cmdId == CommandParser.UP:
            self.controller.turretUp()
            time.sleep(0.05 * times)
            self.controller.turretStop()

        elif cmdId == CommandParser.DOWN:
            self.controller.turretDown()
            time.sleep(0.05 * times)
            self.controller.turretStop()

        elif cmdId == CommandParser.LEFT:
            self.controller.turretLeft()
            time.sleep(0.1 * times)
            self.controller.turretStop()

        elif cmdId == CommandParser.RIGHT:
            self.controller.turretRight()
            time.sleep(0.1 * times)
            self.controller.turretStop()

        elif cmdId == CommandParser.FIRE:
            self.controller.turretFire()

        # Tracking commands
        elif cmdId == CommandParser.TRACK:
            color = args[0]
            self.initializeTracking(color)
        elif cmdId == CommandParser.UNTRACK:
            self.deinitializeTracking()
            
        return "done"

    pass



################################################################################
# Main script

if __name__ == "__main__":

    # import sys
    # cp = CommandParser()
    # (cmd, args) = cp.parse(sys.argv[1])
    # print cp.getCommandName(cmd), args

    server = SkypeServer()
    server.start()
