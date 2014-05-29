#This code might be used to add/remove tasks within the control system
from crontab import CronTab

cron = CronTab(user='pi') #'root' also works if needed
cmd = 'python /home/pi/Desktop/RSSreader/test.py'

#ADD JOB
job = cron.new(command=cmd,comment='RANDOM COMMENT') #Make new cron job
job.minute.on(5)
job.hour.on(6)


#REMOVE JOB
#job = cron.find_comment('SomeID') #Find all jobs with comment '12345'
#cron.remove(job[0])

#SEND TO CRONTAB
cron.write() #Write cronjob to crontab
print cron.render() #Display updated cron file


### OTHER COMMANDS ###
#job = cron.find_comment('12345') #Find all jobs with comment '12345'
#job[0].enable(False) #Disable the first job in list of jobs w/ comment '12345'
#job[0].enable() #Enable first job in list of jobs w/ comment '12345'

#job = cron.find_command(cmd)
#if len(job) > 0:
#    cron.remove_all(cmd)
#cron.write()

#job.minute.every(1) #Set cron job to go off every 1 minute


