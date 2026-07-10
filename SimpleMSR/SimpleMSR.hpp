//
//  SimpleMSR.hpp
//  SimpleMSR
//
//  Created by Park Ju Hyung on 30/06/2018.
//  Copyright © 2018 Park Ju Hyung. All rights reserved.
//

#ifndef SimpleMSR_h
#define SimpleMSR_h

#include <IOKit/IOService.h>

/*
 * For undervolting, consult
 * https://github.com/mihic/linux-intel-undervolt
 */

struct msr_pair {
    const char *name;
    uint32_t index;
    uint64_t value;
};

static const struct msr_pair msr_pairs[] = {
    {
        .name = "MSR_POWER_CTL",
        .index = 0x1FC,
        .value = 0
    },
    #if 0
    {
        .name = "MSR_PKG_POWER_LIMIT (P1 9 P2 15 and other parameters)",
        .index = 0x610,
        .value = 0x42807800dd8048
    },
    {
        .name = "CPU -40 mwat mbox",
        .index = 0x150,
        .value = 0x80000011fb000000
    },
    {
        .name = "GPU -40 mwat mbox",
        .index = 0x150,
        .value = 0x80000111fb000000
    },
    #endif
};

class SimpleMSR : public IOService
{
    OSDeclareDefaultStructors(SimpleMSR)
public:
    virtual bool init(OSDictionary *dictionary = 0);
    virtual void free(void);
    virtual IOService *probe(IOService *provider, SInt32 *score);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    virtual IOReturn setPowerState(unsigned long whichState, IOService * whatDevice);
private:
    void setMSR();
};


#endif /* SimpleMSR_h */
