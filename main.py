### Made by Ward Prescott Summer 2014
### Built for Python 3.4 with support for Python 2.7
# Ryan was here
import datetime

def importLMP():
	'''This function checks to see if LMP data has been downloaded for the day, and if it has not, it uses the file LMP.py to download it.
	'''

	file = open("LMPLog.txt", 'r')
	logText = file.read()
	file.close()

	if logText[0:10] != str(datetime.date.today()):
		print("Data not taken today.  Downloading data from PJM.")
		import lmp
		file = open("LMPLog.txt", 'w')
		file.write(str(datetime.date.today()) + logText[0:100])
		file.close()

importLMP()
