import pymongo as pm
import datetime as dt
import os
import shutil

dbStr = 'mongodb://gongcq:gcq@localhost:27017/text'
mc = pm.MongoClient(dbStr)
db = mc.text
col = db['section']

# # all
# sections = col.find({'time': {'$gte': dt.datetime(2017,12,1), '$lte': dt.datetime(2017,12,31)}})
# file = open('./corpus/corpus.txt', 'w')
# for sec in sections:
#     if sec['masterId'] == '':
#         file.writelines(sec['title'])
#         file.writelines(sec['secTitle'])
#         file.writelines(sec['content'])

# for each day and news
folder = os.path.join('.', 'corpus', 'for_each_day')
beginTime = dt.datetime(2018, 3, 7, 15, 0, 0)
endTime = dt.datetime(2018, 3, 8, 8, 0, 0)
dateStr = endTime.strftime('%Y%m%d')
dateFolder = os.path.join(folder, dateStr)
if os.path.exists(dateFolder):
    shutil.rmtree(dateFolder)
os.mkdir(dateFolder)
sections = col.find({'time': {'$gte': beginTime, '$lte': endTime}})
for sec in sections:
    if sec['masterId'] == '':
        file = open(os.path.join(folder, dateStr,
                                 str(sec['time']) + ' ' +
                                 sec['secTitle'][0: min(20, len(sec['secTitle']))].replace('/', '_') + '.txt'),
                                 'w')
        file.writelines(sec['title'] + '\n')
        file.writelines(sec['secTitle'] + '\n')
        file.writelines(sec['content'])
