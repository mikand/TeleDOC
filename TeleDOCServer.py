#!/usr/bin/env python

import Skype4Py
import time


class CommandParser(object):
    
    numCommands = 7
    (UP, DOWN, LEFT, RIGHT, FIRE, HELP, UNKNOWN) = range(0,7)
    (INT) = range(0, 1)

    def __init__(self):
        self.aliases = {}
        self.aliases[self.UP] = ["up", "u"]
        self.aliases[self.DOWN] = ["down", "d"]
        self.aliases[self.LEFT] = ["left", "l"]
        self.aliases[self.RIGHT] = ["right", "r"]
        self.aliases[self.FIRE] = ["fire", "f"]
        self.aliases[self.HELP] = ["help", "h"]
        self.aliases[self.UNKNOWN] = []

        self.params = {}
        self.params[self.UP] = [self.INT]
        self.params[self.DOWN] = [self.INT]
        self.params[self.LEFT] = [self.INT]
        self.params[self.RIGHT] = [self.INT]
        self.params[self.FIRE] = [self.INT]
        self.params[self.HELP] = []
        self.params[self.UNKNOWN] = []

        self.usage = {}
        self.usage[self.UP] = "Moves the thing up"
        self.usage[self.DOWN] = "Moves the thing down"
        self.usage[self.LEFT] = "Moves the thing left"
        self.usage[self.RIGHT] = "Moves the thing right"
        self.usage[self.FIRE] = "Scatenate l'inferno"
        self.usage[self.HELP] = "Ths help message"
        self.usage[self.UNKNOWN] = ""

        return


    def parse(self, cmd):
        
        # Clean a bit the string
        cmd = cmd.lower()
        cmd = cmd.strip()

        cmds = cmd.split()

        for i in range(0, self.numCommands):
            for alias in self.aliases[i]:
                if cmds[0].startswith(alias):
                    args = []
                    c = 0
                    for param in self.params[i]:
                        c = c + 1

                        if c >= len(cmds):
                            break

                        if self.INT == param:
                            args.append(int(cmds[c]))
                    return (i, args)
        return (self.UNKNOWN, [])


    def getHelp(self):
        res = "Available commands:\n"
        for i in range(0, self.numCommands):
            first = True
            for alias in self.aliases[i]:
                if not first:
                    res += " | "

                res += alias

                if first:
                    first = False

            res += "   "

            for param in self.params[i]:
                if param == self.INT:
                    res += "[number] "

            res += "   "
            res += self.usage[i]

            res += "\n"

        return res


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
            print "Exiting MissleBot"
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
            return self.parser.getHelp()

        # Times to repeat
        times = 1
        if len(args) > 0:
            times = args[0]

        # Real commands
        if cmdId == CommandParser.UP:
            pass
        elif cmdId == CommandParser.DOWN:
            pass
        elif cmdId == CommandParser.LEFT:
            pass
        elif cmdId == CommandParser.RIGHT:
            pass
        elif cmdId == CommandParser.FIRE:
            pass

        return "done"

    pass



################################################################################
# Main script

if __name__ == "__main__":
    server = SkypeServer()
    server.start()

