#include "disk_usage.hh"

int getSysDiskUsage() {
        FILE * fp;
        char filesystem[SYS_DISK_NAME_LEN],available[SYS_DISK_NAME_LEN],use[SYS_DISK_NAME_LEN],mounted[SYS_DISK_NAME_LEN],buf[SYS_DISK_BUFF_LEN];
        double used,blocks,used_rate;
        fp=popen("df","r");
        fgets(buf,SYS_DISK_BUFF_LEN,fp);
        double dev_total = 0,dev_used = 0;
        while(6 == fscanf(fp,"%s %lf %lf %s %s %s",filesystem,&blocks,&used,available,use,mounted)) {
                dev_total += blocks;
                dev_used += used;
        }

        used_rate = (dev_used/dev_total)*SYS_100_PERSENT;
        pclose(fp);
        return used_rate;
}