## Written by Ward Prescott
## BUSERT
## Summer 2014
##

import requests
import time
import datetime
import re
import xlsxwriter
from datetime import date

API_KEY= 'd7f06f2a02c528b6345bfb52348a2d71'

USER_ID= '4d6a59334d5459320a'

EHOUR=23



HOURSINDAY = 24
MINSINHOUR = 60

#Starting Day and Time
sYear=2014
sMon=1
sDay=8
sHour=1

eYear = 2014
eMon = 5
eDay = 21
ehour = 23

STARTDATE = datetime.date(sYear, sMon, sDay)

startDate = datetime.datetime(sYear, sMon, sDay, sHour)
endDate =  datetime.datetime(eYear, eMon, eDay, sHour)
difference = endDate - startDate
diff = difference.days
print(diff)

#Converting to Epoch
S = datetime.datetime(sYear, sMon, sDay, sHour)
E= datetime.datetime(sYear,sMon,sDay,EHOUR)

Start_time=time.mktime(S.timetuple())

End_time=time.mktime(E.timetuple())

file = open("datafile.txt", 'w')

## ------ API Queries, takes some time so be patient ------- ##
for x in range (0,diff):
    Start_time += 86400
    End_time += 86400
    parameters = {'start_at':Start_time,'end_at':End_time,'key': API_KEY, 'user_id':USER_ID}
    r=requests.get('https://api.enphaseenergy.com/api/v2/systems/297704/stats', params = parameters)
    file.write(r.text)
    time.sleep(6)

file.close()

## ------ Worksheet Setup ----- ##
workbook = xlsxwriter.Workbook('SolarOutput.xlsx')
worksheet = workbook.add_worksheet()
worksheet2 = workbook.add_worksheet()
worksheet3 = workbook.add_worksheet()

date = STARTDATE

#Setup rows
for column in range(diff):
        worksheet.write(0, column + 1, str(date))
        worksheet2.write(0, column + 1, str(date))
        worksheet3.write(0, column + 1, str(date))
        date += datetime.timedelta(days=1)

colTime = 00
colTime2 = 00
for row in range(HOURSINDAY * MINSINHOUR / 5):
        if colTime < 10:
                worksheet.write(row + 1, 0, str(colTime2) + ":0" + str(colTime))
                worksheet2.write(row + 1, 0, str(colTime2) + ":0" + str(colTime))
        else:
                worksheet.write(row + 1, 0, str(colTime2) + ":" + str(colTime))
                worksheet2.write(row + 1, 0, str(colTime2) + ":" + str(colTime))
        colTime += 5
        if colTime == 60:
                colTime2 += 1
                colTime = 0

##Import the data and make it into a list of values

file = open('datafile.txt', 'r')

rawData = file.read()

file.close()

timesraw = re.findall('[0-9]{10}', rawData)
powerraw = re.findall('powr\"\:\d{1,4}', rawData)
energyraw = re.findall('enwh\"\:\d{1,4}', rawData)
times = [int(s) for s in timesraw]
powerbetter = [s[6:] for s in powerraw]
power = [int(s) for s in powerbetter]
energybetter = [s[6:] for s in energyraw]
energy = [int(s) for s in energybetter]


##Generate power spreadsheet  
startTime = 1389243600
ds = 0

for col in range(diff):
        for row in range(HOURSINDAY * MINSINHOUR / 5):
                if startTime == 1394344800 and ds == 0:
                        startTime -= 3600   #Daylight savings
                        ds = 1
                if startTime in times:
                        index = times.index(startTime)
                        worksheet.write(row + 1, col + 1, power[index])
                else:
                        worksheet.write(row + 1, col + 1, 0)
                startTime += 300

##Generate energy spreadsheet
startTime = 1389243600
ds = 0

for col in range(diff):
        for row in range(HOURSINDAY * MINSINHOUR / 5):
                if startTime == 1394344800 and ds == 0:
                        startTime -= 3600   #Daylight savings
                        ds = 1
                if startTime in times:
                        index = times.index(startTime)
                        worksheet2.write(row + 1, col + 1, energy[index])
                else:
                        worksheet2.write(row + 1, col + 1, 0)
                startTime += 300

##Generate Intuitive Energy spreadsheet
startTime = 1389243600 + 300
ds = 0
clock = 1
addition = ":00"

for row in range(HOURSINDAY):
        worksheet3.write(row + 1, 0, str(clock)+addition)
        clock += 1

runningTotal = 0

for col in range(diff):
        for row in range(HOURSINDAY):
                for count in range(12):
                        if startTime == 1394344800 and ds == 0:
                                startTime -= 3600   #Daylight savings
                                ds = 1
                        if startTime in times:
                                index = times.index(startTime)
                                runningTotal += energy[index]
                        startTime += 300

                worksheet3.write(row + 1, col + 1, runningTotal)
                runningTotal = 0

workbook.close()
