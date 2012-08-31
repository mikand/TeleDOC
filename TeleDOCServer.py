#!/usr/bin/env python

# Copyright 2012, Pisellino & Aurelio Pam Pam ;-)


import Skype4Py
import time
import argparse
import usb.core



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
        self.dev = usb.core.find(idVendor=0x2123, idProduct=0x1010)
        if self.dev is None:
            raise ValueError('Launcher not connected!')
         
        if self.dev.is_kernel_driver_active(0) is True:
            self.dev.detach_kernel_driver(0)

        self.dev.set_configuration()

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
    (UP, DOWN, LEFT, RIGHT, FIRE, HELP, UNKNOWN) = range(0,7)

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

        # create the parser for the "fire" command
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
            else:
                return (self.UNKNOWN, [])

        except ExitException as e:
            return (self.HELP, [e.message])


    def getComamndName(self, cmdId):
        if cmdId == self.UP:
            return "UP"

        if cmdId == self.DOWN:
            return "DOWN"

        if cmdId == self.LEFT:
            return "LEFT"

        if cmdId == self.RIGHT:
            return "RIGHT"

        if cmdId == self.FIRE:
            return "FIRE"

        if cmdId == self.HELP:
            return "HELP"
        
        return "UNKNOWN"

    pass



class SkypeServer():
    
    def __init__(self):
        # Create an instance of the Skype class.
        self.skype = Skype4Py.Skype(Transport='x11')
        self.parser = CommandParser()
        self.controller = LauncherController()
        return


    def start(self):
        # Connect the Skype object to the Skype client.
        self.skype.Attach()

        # Register callbacks
        self.skype.OnMessageStatus = self.onMessageStatus
        self.skype.OnCallStatus = self.onCallStatus

        # Main loop
        try:
            while self.skype.AttachmentStatus == Skype4Py.apiAttachSuccess:
                time.sleep(5)
        except KeyboardInterrupt:
            print "\nExiting TeleDOC MissileLauncher\n"
        return


    def onCallStatus(self, call, status):
        # Respond automatically with video
        if status == Skype4Py.clsRinging:
            print "Accepting call from %s" % call.PartnerHandle
            call.Answer()
            call.StartVideoSend()

    def onMessageStatus(self, message, status):
        # Dispatch all messages
        if status == Skype4Py.cmsReceived:
            # I received a message
            res = self.doComamnd(message.Body)
            message.Chat.SendMessage(res)


    def doComamnd(self, cmd):
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

        elif cmdId == CommandParser.DOWN:
            self.controller.turretDown()

        elif cmdId == CommandParser.LEFT:
            self.controller.turretLeft()

        elif cmdId == CommandParser.RIGHT:
            self.controller.turretRight()

        elif cmdId == CommandParser.FIRE:
            self.controller.turretFire()

        return "done"

    pass



################################################################################
# Main script

if __name__ == "__main__":

    # import sys
    # cp = CommandParser()
    # (cmd, args) = cp.parse(sys.argv[1])
    # print cp.getComamndName(cmd), args

    server = SkypeServer()
    server.start()

