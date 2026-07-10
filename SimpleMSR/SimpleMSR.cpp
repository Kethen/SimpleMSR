//
//  SimpleMSR.c
//  SimpleMSR
//
//  Created by Park Ju Hyung on 30/06/2018.
//  Copyright © 2018 Park Ju Hyung. All rights reserved.
//

#include <IOKit/IOLib.h>
#if TARGET_CPU_X86_64
#include <i386/proc_reg.h> // For msr operators
#endif
#include "SimpleMSR.hpp"

#define kPowerStates 2
#define kIOPMPowerOff 0

// Define the driver's superclass.
#define super IOService

OSDefineMetaClassAndStructors(SimpleMSR, IOService)

#define TAG "SimpleMSR: "

bool SimpleMSR::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    IOLog(TAG "Initializing\n");
    return result;
}

void SimpleMSR::free(void)
{
    IOLog(TAG "Freeing\n");
    super::free();
}

IOService *SimpleMSR::probe(IOService *provider,
                                    SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog(TAG "Probing\n");
    
    return result;
}

bool SimpleMSR::start(IOService *provider)
{
    bool result = super::start(provider);
    IOLog(TAG "Starting\n");
    PMinit();
    provider->joinPMtree(this);
    static IOPMPowerState myPowerStates[kPowerStates] = {
        {1, kIOPMPowerOff, kIOPMPowerOff, kIOPMPowerOff, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    registerPowerDriver (this, myPowerStates, kPowerStates);
    
    return result;
}

void SimpleMSR::stop(IOService *provider)
{
    IOLog(TAG "Stopping\n");
    PMstop();
    super::stop(provider);
}

static void sync_610(uint64_t value){
    // as discussed at https://github.com/daliansky/XiaoMi-Pro-Hackintosh/issues/174
    const uint64_t target = 0xfed159a0;
    IOMemoryDescriptor *target_md_wr = IOMemoryDescriptor::withPhysicalAddress(target, sizeof(uint64_t), kIODirectionOut);
    IOMemoryDescriptor *target_md_rd = IOMemoryDescriptor::withPhysicalAddress(target, sizeof(uint64_t), kIODirectionOut);
    uint64_t cur_value = 0;
    IOByteCount bytes_written = 0;
    IOByteCount bytes_read = 0;
    if (target_md_wr == NULL){
        IOLog(TAG "%s: failed creating IOMemoryDescriptor for writing\n", __func__);
        goto cleanup;
    }
    if (target_md_rd == NULL){
        IOLog(TAG "%s: failed creating IOMemoryDescriptor for reading\n", __func__);
        goto cleanup;
    }
    
    bytes_read = target_md_wr->readBytes(0, &cur_value, sizeof(cur_value));
    if (bytes_read != sizeof(cur_value)){
        IOLog(TAG "%s: failed reading bytes before writing\n", __func__);
        goto cleanup;
    }
    IOLog(TAG "%s: setting 0x%llx from 0x%llx to 0x%llx\n", __func__, target, cur_value, value);

    bytes_written = target_md_wr->writeBytes(0, &value, sizeof(value));
    if (bytes_written != sizeof(value)){
        IOLog(TAG "%s: failed writing bytes\n", __func__);
        goto cleanup;
    }

    bytes_read = target_md_rd->readBytes(0, &cur_value, sizeof(cur_value));
    if (bytes_read != sizeof(cur_value)){
        IOLog(TAG "%s: failed reading bytes after writing\n", __func__);
        goto cleanup;
    }

    IOLog(TAG "%s: 0x%llx is now 0x%llx\n", __func__, target, cur_value);

    cleanup:
    if (target_md_rd){
        target_md_rd->release();
    }
    if (target_md_wr){
        target_md_wr->release();
    }
}

void SimpleMSR::setMSR()
{
#if TARGET_CPU_X86_64
    // Hardcoded atm
    int i;
    for (i = 0; i < sizeof(msr_pairs) / sizeof(msr_pairs[0]);i++){
        IOLog(TAG "Changing %s", msr_pairs[i].name);
        IOLog(TAG "Read MSR 0x%x = 0x%llx\n", msr_pairs[i].index, rdmsr64(msr_pairs[i].index));
        wrmsr64(msr_pairs[i].index, msr_pairs[i].value);
        IOLog(TAG "Changed MSR 0x%x to 0x%llx\n", msr_pairs[i].index, rdmsr64(msr_pairs[i].index));
        if (msr_pairs[i].index == 0x610){
            sync_610(msr_pairs[i].value);
        }
    }
    
    IOLog(TAG "MSR all set\n");
#endif
}

IOReturn SimpleMSR::setPowerState ( unsigned long whichState, IOService * whatDevice )
{
    if (kIOPMPowerOff != whichState) {
        IOLog(TAG "Waking up\n");
        this->setMSR();
    }
    return kIOPMAckImplied;
}
