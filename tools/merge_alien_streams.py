"""Private asset experiment: merge existing animation records, never generate clips."""
import struct

def channel_records(blob):
    entries(blob,True) # Check all bounds, including duplicate hash records.
    _,n,_,record_at,stride=struct.unpack_from('>5I',blob,32)
    base=struct.unpack_from('>I',blob,28)[0]
    return [blob[base+offset:base+offset+size] for _,size,offset in
            (struct.unpack_from('>3I',blob,32+record_at+i*stride) for i in range(n))]

def animation_time_range(clip,channels):
    # Original resident initializer 0x8219ED50; streamed loading only relocates
    # pointers and expects these time bounds to have already been baked.
    begin,end=10000000.0,-10000000.0
    count=struct.unpack_from('>I',clip,72)[0]
    assert 216+count*12<=len(clip)
    for at in range(216,216+count*12,4):
        index=struct.unpack_from('>I',clip,at)[0]
        if index==0xffffffff:continue
        channel=channels[index]
        keys=struct.unpack_from('>H',channel,4)[0];stride=channel[1]*4
        assert keys>0 and stride>0
        if channel[0]==16:
            first=channel[15]*160.0;last=channel[15+(keys-1)*stride]*160.0
        else:
            first=struct.unpack_from('>f',channel,8)[0]
            last=struct.unpack_from('>f',channel,8+(keys-1)*stride)[0]
        import math
        assert math.isfinite(first) and math.isfinite(last)
        begin=min(begin,first);end=max(end,last)
    return begin,end

def streamed_clip(clip,channels,reconstruct_timing=True):
    # Resident clips store indices into section 9, including duplicate keys.
    # Streamed clips store byte offsets to inline channels instead.
    bones=struct.unpack_from('>I',clip,76)[0]
    assert len(clip)==216+bones*12, (len(clip),bones)
    out=bytearray(clip);mapped={}
    if reconstruct_timing:struct.pack_into('>2f',out,64,*animation_time_range(clip,channels))
    for at in [80]+list(range(216,len(clip),4)):
        index=struct.unpack_from('>I',clip,at)[0]
        if index==0xffffffff:continue
        assert index<len(channels), (index,len(channels))
        if index not in mapped:
            out.extend(b'\0'*((-len(out))%4));mapped[index]=len(out)
            out.extend(channels[index])
        struct.pack_into('>I',out,at,mapped[index])
    return bytes(out)

def entries(blob,allow_duplicates=False):
    assert blob[:8]==bytes.fromhex('7661707300010003')
    count=struct.unpack_from('>I',blob,8)[0]
    base=struct.unpack_from('>I',blob,28)[0]
    buckets,n,bucket_at,record_at,stride=struct.unpack_from('>5I',blob,32)
    assert n==count and stride==12 and 0<buckets<100000
    assert 32+record_at+n*12<=base<=len(blob)
    result={}
    for i in range(n):
        key,size,offset=struct.unpack_from('>3I',blob,32+record_at+i*12)
        assert base+offset+size<=len(blob), (hex(key),base,offset,size,len(blob))
        assert allow_duplicates or key not in result, ('duplicate key',hex(key))
        if key not in result:result[key]=blob[base+offset:base+offset+size]
    return result

def merge_streams(original,archives,reconstruct_timing=True):
    assert struct.unpack_from('>I',original,20)[0]==13
    records=entries(original,True);old=dict(records);added={}
    for name,blob in archives:
        sections=entries(blob);section=sections.get(13)
        if section is None:continue
        assert struct.unpack_from('>I',section,20)[0]==13
        channels=channel_records(sections[9])
        count=0
        for key,value in entries(section,True).items():
            if key not in records:records[key]=streamed_clip(value,channels,reconstruct_timing);count+=1
        added[name]=count
    n=len(records);buckets=(n+3)//4*4
    alignment=1<<struct.unpack_from('>H',original,12)[0]
    assert 16<=alignment<=65536
    ordered=sorted(records,key=lambda key:(key%buckets,key))
    dictionary=bytearray(original[32:64])
    struct.pack_into('>5I',dictionary,0,buckets,n,32,32+buckets*4,12)
    cursor=0
    for bucket in range(buckets):
        while cursor<n and ordered[cursor]%buckets<bucket:cursor+=1
        dictionary.extend(struct.pack('>I',cursor))
    payload=bytearray()
    for key in ordered:
        payload.extend(b'\0'*((-len(payload))%alignment))
        dictionary.extend(struct.pack('>3I',key,len(records[key]),len(payload)))
        payload.extend(records[key])
    base=(32+len(dictionary)+alignment-1)//alignment*alignment
    header=bytearray(original[:32]);struct.pack_into('>I',header,8,n)
    struct.pack_into('>I',header,16,len(payload));struct.pack_into('>I',header,28,base)
    result=bytes(header+dictionary)+b'\0'*(base-32-len(dictionary))+bytes(payload)
    decoded=entries(result)
    assert decoded==records and all(decoded[k]==v for k,v in old.items())
    return result,dict(original_records=len(old),merged_records=n,added_by_archive=added,
        original_records_preserved=True,gameplay_verified=False)
