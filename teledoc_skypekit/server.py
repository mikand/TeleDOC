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
renderer = teledoc.TeledocRenderer(100, 30, 5)

rendered = False

MySkype = Skype.GetSkype(keypair.keyFileName)
MySkype.Start()


def OnAvailableDeviceListChange(self):
        global devices
        global renderer

        devices = self.GetAvailableVideoDevices()
        if devices is None or len(table[0]) == 0:
                print "No video devices found"
                devices = None
        else:
                video = self.getPreviewVideo(Skype.Video.MEDIATYPE['MEDIA_VIDEO'], devices[0][0], devices[0][1])
                video.SetRemoteRendererId(renderer.getKey())
                rendered = True
                

Skype.OnAvailableDeviceListChange = OnAvailableDeviceListChange


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
                sleep(1)
except KeyboardInterrupt:
        print "\nExiting...\n"


# CV loop
try:
        while True:
                if renderer.newFrameAvailable():
                        print renderer.getCurrentPosition()
                sleep(1)
except KeyboardInterrupt:
        print "\nExiting...\n"

MySkype.stop()
