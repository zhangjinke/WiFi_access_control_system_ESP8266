import httplib, urllib
import json
import time
import os.path as op

reset_reason = ["REASON_DEFAULT_RST",
                "REASON_WDT_RST",
                "REASON_EXCEPTION_RST",
                "REASON_SOFT_WDT_RST",
                "REASON_SOFT_RESTART",
                "REASON_DEEP_SLEEP_AWAKE"]
    
rst_info_item = ["reason",
                 "exccause",
                 "epc1",
                 "epc2",
                 "epc3",
                 "excvaddr",
                 "depc"]


httpClient = None
def esp_server_http_request(ip_addr,devkey,path,method,data,tout):
    try:
        headers = {"Content-type": "application/json", "Accept": "text/plain","Authorization":"token "+devkey}
        httpClient = httplib.HTTPConnection(ip_addr, timeout=tout)
    
        httpClient.request(method, path, data, headers)
        response = httpClient.getresponse()
        print response.status
        data = response.read()
        return data
    except Exception, e:
        print e
    finally:
        if httpClient:
            httpClient.close()   


def get_debug_info(ip_addr,devkey, offset=None, raw_count=None,start=None,end=None,tout=3):
    path="/v1/device/debugs/?"
    if not offset == None:
        path += "offset=%d&"%offset
    if not raw_count == None:
        path += "raw_count=%d&"%raw_count
    if not start == None and not end == None and end>start:
        path += "start=%s&end=%s&"%(start,end)

    path = path.strip("&").strip('?')
    
    print "path:",path
    res = esp_server_http_request(ip_addr=ip_addr,devkey=devkey,path=path,method="GET",data=None,tout=tout)
    #print "test :res:"
    #print res
    return res
    
            
def get_debug_info_list(mac_list,record_file,start,end,offset,raw_cnt,ip_addr):
    f=open(record_file,'r')
    lines = f.readlines()
    f.close()
    
    log = ""
    num = 0
    for mac_addr in mac_list:
        for line in lines:
            ltmp = line.strip("\n").split(',')
            mac_rec = ltmp[0]
            dev_key = ltmp[3]
            if mac_addr.upper() in mac_rec:
                num+=1
                print "------------"
                print "mac:",mac_rec
                print "devkey:",dev_key

                log+="====================\r\n"
                log+="DEV MAC: %s\r\n"%mac_rec
                log+="DEV KEY: %s\r\n"%dev_key
                log+="-----------\r\n"
                
                debug_info = get_debug_info(ip_addr,dev_key, offset, raw_count,start,end,10)
                #print debug_info
                debug_info = json.loads(debug_info)
                
                debug_list = debug_info["debugs"]
                for d_dict in debug_list:
                    print "---------------------"
                    print d_dict['updated']
                    info = d_dict['message'].strip('[').strip(']').split(',')
                    info = [int(x,16) for x in info]
                    print "reset reason: %d %s"%(info[0],reset_reason[info[0]])
                    log+= d_dict['updated']+"\r\n"
                    log+= "reset reason: %d %s\r\n"%(info[0],reset_reason[info[0]])
                    if info[0] == 2:
                        print "Fatal exception (%d):\n"%info[1]
                        log+= "Fatal exception (%d):\r\n"%info[1]
                    print "epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n"%(info[2],info[3],info[4],info[5],info[6])
                    log+= "epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\r\n"%(info[2],info[3],info[4],info[5],info[6])
    if log == '':
        print "NO EXCEPTION FOUND..."
    else:
        print "log here:\r\n"
        log = "**************\r\n"+'DEV NUM:%d\r\n'%num+"*************\r\n"+log
        #print log
        f = open("exception_log.txt",'w')
        f.write(log)
        f.close()
        
                    
                
        
            
if __name__=="__main__":
    mac_list = ['9ec4fc','a0584e','9ecbba','9e546e','9ec254','9df489','9ec2a2','a1068f','a107c2',
                'a106d7','a10927','a1090c','a0bc26','9e5a0b','a0a563','9ff9ec','a04c37','a1062d','a107f2']
    #mac_list = ['a04c37']
    ip_addr = "115.29.202.58"
    offset = 0
    raw_count = 1000
    start = "2015-09-18 1:00:00"
    end = "2015-09-18 23:00:00"
    
    get_debug_info_list(mac_list, 'dev_key_list.csv',start,end,offset,raw_count,ip_addr)
    
    