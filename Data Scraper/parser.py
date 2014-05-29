## Written by Ward Prescott
## BUSERT
## Summer 2014
##


import xlsxwriter
import time
import datetime
import re

## ------ CONSTANTS ----------- ##
STARTDATE = datetime.datetime(2014, 1, 9)


## ------ Worksheet Setup ----- ##
workbook = xlsxwriter.Workbook('SolarOutput.xlsx')
worksheet = workbook.add_worksheet()
worksheet2 = workbook.add_worksheet()
worksheet3 = workbook.add_worksheet()
worksheet4 = workbook.add_worksheet()

date = STARTDATE

#Setup rows
for column in range(128):
        worksheet.write(0, column + 1, str(date))
        worksheet2.write(0, column + 1, str(date))
        worksheet3.write(0, column + 1, str(date))
        worksheet4.write(0, column + 1, str(date))
        date += datetime.timedelta(days=1)

colTime = 00
colTime2 = 00
for row in range(288):
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

for col in range(128):
        for row in range(288):
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

for col in range(128):
        for row in range(288):
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

for row in range(24):
        worksheet3.write(row + 1, 0, str(clock)+addition)
        clock += 1

runningTotal = 0

for col in range(128):
        for row in range(24):
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


##Generate Second Intuitive Energy spreadsheet
startTime = 1389243600
ds = 0
clock = 15
clock2 = 0
addition = ":00"

for row in range(24 * 4):
        worksheet4.write(row + 1, 0, str(clock2)+str(clock))
        clock += 15
        if clock == 60:
                clock2 += 1
                clock = 0

runningTotal = 0

for col in range(128):
        for row in range(24*4):
                for count in range(3):
                        if startTime == 1394344800 and ds == 0:
                                startTime -= 3600   #Daylight savings
                                ds = 1
                        if startTime in times:
                                index = times.index(startTime)
                                runningTotal += energy[index]
                        startTime += 300

                worksheet4.write(row + 1, col + 1, runningTotal)
                runningTotal = 0


workbook.close()
