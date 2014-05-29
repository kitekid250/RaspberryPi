#This file should run daily and does the following:
#Removes day3.txt and renames day1 and day2 to reflect the fact that another day has passed
#Downloads tomorrow's projected LMP from PJM
#Parses out PPL's projected values
#Updates day1.txt's values to reflect tomorrow's prices
#Updates google spreadsheet so that website's graphs will reflect the changes

import feedparser
import urllib
import time
import csv
import os
import sys
##import gdata.spreadsheet.service
##import gspread

def rename_files():
        print ("Rename/Remove old files")
        #Removes day3 and renames days 1 and 2 so that day1=2 and day2=3
        try:
                os.remove("day3.txt")
        except OSError:
                print ("There was no file named day3.txt")
        try:
                os.rename("day2.txt", "day3.txt")
        except OSError:
                print ("There was no file named day2.txt")
        try:
                os.rename("day1.txt", "day2.txt")
        except OSError:
                print ("There was no file named day1.txt")

def download_lmp():
        #Finds the correct file for tomorrow's LMP
        print ("Getting file URL")
        d = feedparser.parse("http://www.pjm.com/pub/account/lmpda/rss.xml")
        print ("URL: " + d.entries[Day]['link'])      # Print the first entry and its link
        url = d.entries[Day]['link']
        downloadName = 'temp.csv' #d.entries[Day]['title']#+"-"+time.asctime()
        print(downloadName)
        print(type(downloadName))
        #parsedName = "output-" + time.asctime()+".txt"
        #Downloads file
        webFile = urllib.request.urlopen(url)
        print(webFile)
        localFile = open(downloadName,'w')
        localFile.write(str(webFile.read()))
        webFile.close()
        localFile.close()
        print ("File download complete.")
        return downloadName

def parse_file(downloadName, parsedName):
        #Searches file for PPL's data and saves it to a file
        print ("Parsing file...")
        file1 = open(downloadName, 'rt')
        file2 = open(parsedName, 'w+')
        outString = ""
        for row in csv.reader(file1):
                if str(row).find("PPL"):
                        indexRow = row.index("PPL")
                        print(indexRow)
                        print(row[indexRow])
                        count = 5
                        #print(str(row))
                        while count < 77 :
                                outString += (row[indexRow+count]) + " "
                                file2.write(str(row[indexRow+count]) + " ")
                                count = count + 3
                        file2.write("\n")
                        break
        file1.close()
        file2.close()
        print ("Parsing complete.")
        return outString

def remove_file(fileName):
        try:
                #os.remove(fileName)
                print ("webFile removed")
        except OSError:
                print ("there was no file with that name")

##def update_spreadsheet(outString):
##      print ("Updating spreadsheet...")
##      outString = outString.split()
##      email = 'busert123@gmail.com'
##      password = 'treehugger'
##      gc = gspread.login(email, password)
##      spreadsheet = gc.open("BUSERT")
##      worksheet = spreadsheet.sheet1
##      val = worksheet.acell('B13').value
##      cnt = 0
##      while (cnt < 24):
##              today = str(worksheet.cell(3, cnt+1).value)
##              yesterday = str(worksheet.cell(4, cnt+1).value)
##              worksheet.update_cell(3, cnt+1, outString[cnt])
##              worksheet.update_cell(4, cnt+1, today)
##              worksheet.update_cell(5, cnt+1, yesterday)
##              cnt = cnt + 1
##      print ("Update Complete")

#Specify which proj. LMP data to download
#0 is tomorrow, 1 is today, 2 is yesterday, 3 is day before yesterday...etc
Day = 0

rename_files()
downloadName = download_lmp()

parsedName = "day1.txt"
outString = parse_file(downloadName, parsedName)

remove_file(downloadName) #Removes downloaded lmp data file because it is no longer needed
print ("Key string: " + outString)#Displays tomorrow's projected hourly LMP

##update_spreadsheet(outString) #Modifies google spreadsheet to reflect the new day's data.
print ("Program complete")
