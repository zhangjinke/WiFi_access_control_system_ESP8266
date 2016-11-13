import struct


light_dev_num = 10



def gen_mac_list(magic_t,espnow_key,file_path,channel_list):
    f=open(file_path,'r')
    lines = f.readlines()
    f.close()
    mac_list = bytearray("")
    
    mac_num = len(lines)
    print "mac num:",mac_num
    if mac_num>light_dev_num:
        mac_num = light_dev_num
    
    
    #for line in lines:
    for i in range(light_dev_num):
        try:
            line = lines[i]
        except:
            line = ''
        line = line.strip('\n').strip(",")
        if not line == '':
            print "line:",line
            d_mac = int(line,16)
            mac_addr = [(d_mac>>(8*i))&0xff for i in range(5,-1,-1) ]
            if mac_addr[0] == 0x18:
                mac_addr[0] = 0x1a
        else:
            mac_addr = [0xff]*6
            
        
        mac_addr = bytearray(mac_addr)
        print mac_addr
        mac_list.extend(mac_addr)
        #mac_addr = struct.pack("<BBBBBB",*mac_addr)
        #print mac_addr
        #mac_list
        
    
    data_full = bytearray("")
    
    magic = struct.pack("<L",magic_t)
    print "magic:",magic
    data_full.extend(magic) 
    #print "-------------"
    #print "len1:",len(data_full)
    
    
    mac_num = struct.pack("<H",mac_num)
    
     
    #data_full.extend( bytearray([mac_num&0xff]))
    data_full.extend(mac_num)
    #print "-------------"
    #print "len2:",len(data_full)   
    #for byte in data_full:
        #print "%02x "%byte
    #print "-------------------"
    data_full.extend(mac_list)
    
    
    #espnow_key = [0x10,0xfe,0x94, 0x7c,0xe6,0xec,0x19,0xef,0x33, 0x9c,0xe6,0xdc,0xa8,0xff,0x94, 0x7d]
    data_full.extend(bytearray(espnow_key))
    

    
    #channel_list = [1]*light_dev_num
    data_full.extend(bytearray(channel_list))
    
    
    
    print "-------------"
    print "len3:",len(data_full)   
    for byte in data_full:
        print("%02x "%byte),
    print "-------------------"    
    
    
    
    bin_name = file_path.split('.')[0]+".bin"
    f = open(bin_name,'wb')    
    #f.write(mac_list) 
    f.write(data_full)
    f.close()
        
        







if __name__ == "__main__":
    magic = 0x5c5caacc
    espnow_key = [0x10,0xfe,0x94, 0x7c,0xe6,0xec,0x19,0xef,0x33, 0x9c,0xe6,0xdc,0xa8,0xff,0x94, 0x7d]
    file_path = "light_mac_list_for_controller_demo01.csv"
    channel_list = [1]*light_dev_num
    gen_mac_list(magic,espnow_key,file_path,channel_list)
