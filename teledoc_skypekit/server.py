import sys
import keypair
import teledoc
from time import sleep

sys.path.append(keypair.distroRoot + '/ipc/python')
sys.path.append(keypair.distroRoot + '/interfaces/skype/python')

import Skype


if len(sys.argv) != 3:
	print('Usage: python server.py <skypename> <password>')
	sys.exit()

accountName = sys.argv[1]
accountPsw = sys.argv[2]
loggedIn = False
devices = None
renderer = teledoc.TeledocRenderer(50, 50, 5)

rendered = False

MySkype = Skype.GetSkype(keypair.keyFileName)
MySkype.Start()


def OnAvailableDeviceListChange(self):
        global devices
        global renderer

        devices = self.GetAvailableVideoDevices()
        if devices is None or len(devices[0]) == 0:
                print "No video devices found"
                devices = None
        else:
                print "Video devices found!"
                rendered = True
                

MySkype.OnAvailableDeviceListChange = OnAvailableDeviceListChange


def AccountOnChange (self, property_name):
	global loggedIn
	if property_name == 'status':
		if self.status == 'LOGGED_IN':
			loggedIn = True
			print('Login complete.')

Skype.Account.OnPropertyChange = AccountOnChange

account = MySkype.GetAccount(accountName)

print('Logging in with ' + accountName)
account.LoginWithPassword(accountPsw, False, False)

while loggedIn == False:
	sleep(1)

# Logged

# Main loop
try:
        while not rendered:
                devices = MySkype.GetAvailableVideoDevices()
                print(devices)
                if devices is None or len(devices[0]) == 0:
                        print "No video devices found"
                        devices = None
                else:
                        rendered = True
                sleep(1)

except KeyboardInterrupt:
        print "\nExiting (1)...\n"
        MySkype.stop()
        exit(1)

video = MySkype.GetPreviewVideo('MEDIA_VIDEO', devices[0][0], devices[1][0])
video.SetRemoteRendererId(renderer.getKey())

# CV loop
try:
        while True:
                if renderer.newFrameAvailable():
                        print renderer.getCurrentPosition()
                sleep(0.05)
except KeyboardInterrupt:
        print "\nExiting (2)...\n"
        MySkype.stop()
        exit(1)

MySkype.stop()

