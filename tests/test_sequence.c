#include "demo_sequence.h"
#include "pdo.h"
#include <stdio.h>
static int run_sequence(int visual)
{
    DemoSequence demo={0,0,0}; Joint slave; JointCommand c={0,0,0,8}, received;
    JointFeedback f; uint8_t output[PROJECT_RXPDO_BYTES],input[PROJECT_TXPDO_BYTES]; unsigned i; int result=1;
    joint_init(&slave,0); f=slave.feedback;
    for(i=0;i<6000 && result>0;i++) {
        result=visual ? demo_next_visual(&demo,&f,&c) : demo_next(&demo,&f,&c);
        project_encode_command(output,&c);
        if(!project_decode_command(output,sizeof(output),&received)) return 1;
        joint_receive(&slave,&received,(i+1)*10000);
        joint_update(&slave,(i+1)*10000,1);
        project_encode_feedback(input,&slave.feedback);
        if(!project_decode_feedback(input,sizeof(input),&f) || f.error_code!=slave.error_code) return 1;
    }
    if(result!=0 || demo.reached!=63 || slave.state!=JD_DISABLED) {
        fprintf(stderr,"sequence failed: phase=%u result=%d flags=%u\n",demo.phase,result,demo.reached); return 1;
    }
    printf("PASS: %s master sequence over simulated PDO in %u cycles\n",visual ? "visual" : "standard",i);
    demo.phase=0; demo.ticks=0; f.statusword=8;
    if(demo_next(&demo,&f,&c)!=-1) return 1;
    return 0;
}

int main(void)
{ return run_sequence(0) || run_sequence(1); }
