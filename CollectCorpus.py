import pymongo as pm
import datetime as dt

dbStr = 'mongodb://gongcq:gcq@localhost:27017/text'
mc = pm.MongoClient(dbStr)
db = mc.text
col = db['section']
sections = col.find({'time': {'$gte': dt.datetime(2017,12,1), '$lte': dt.datetime(2017,12,31)}})
file = open('./corpus/corpus.txt', 'w')
for sec in sections:
    if sec['masterId'] == '':
        file.writelines(sec['title'])
        file.writelines(sec['secTitle'])
        file.writelines(sec['content'])