#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <fstream>
#include "gadgets.h"
#include "LRUbptree.hpp"
const int modp1=998244353;
const int modp2=100000007;
#define __File1 "1.txt"
#define __File2 "2.txt"
#define __File3 "3.txt"
#define __File4 "4.txt"
#define __File5 "5.txt"
#define __File6 "6.txt"
#define __File7 "7.txt"
#define __File8 "8.txt"
#define __File9 "9.txt"
#define __File10 "10.txt"
#define time my_time
using namespace std;
const char instruction_name[16][15]={"add_user","login","logout","query_profile","modify_profile","add_train","release_train","query_train","delete_train","query_ticket","query_transfer","buy_ticket","query_order","refund_ticket","clean","exit"};
int to[300][40],instruction_id[300],cnt;
struct date
{
    int month,day;
    date()
    {
        month=0;
        day=0;
    }
    date(int m,int d)
    {
        month=m;
        day=d;
    }
    bool operator <(date &rhs)
    {
        if (month<rhs.month) return true;
        if (month>rhs.month) return false;
        if (day<rhs.day) return true;
        return false;
    }
    bool operator <(const date &rhs)
    {
        if (month<rhs.month) return true;
        if (month>rhs.month) return false;
        if (day<rhs.day) return true;
        return false;
    }
};
bool operator <(const date &rhs1,date &rhs2)
{
    if (rhs1.month<rhs2.month) return true;
    if (rhs1.month>rhs2.month) return false;
    if (rhs1.day<rhs2.day) return true;
    return false;
}
bool operator <(const date &rhs1,const date &rhs2)
{
    if (rhs1.month<rhs2.month) return true;
    if (rhs1.month>rhs2.month) return false;
    if (rhs1.day<rhs2.day) return true;
    return false;
}
int calc_date(date &d)
{
    if (d.month==6) return d.day;
    if (d.month==7) return 30+d.day;
    return 61+d.day;
}
struct time
{
    int hour,minute;
    time()
    {
        hour=0;
        minute=0;
    }
    time(int h,int m)
    {
        hour=h;
        minute=m;
    }
    bool operator <(time &rhs)
    {
        if (hour<rhs.hour) return true;
        if (hour>rhs.hour) return false;
        if (minute<rhs.minute) return true;
        return false;
    }
    bool operator <(const time &rhs)
    {
        if (hour<rhs.hour) return true;
        if (hour>rhs.hour) return false;
        if (minute<rhs.minute) return true;
        return false;
    }
};
bool operator <(const time &rhs1,time &rhs2)
{
    if (rhs1.hour<rhs2.hour) return true;
    if (rhs1.hour>rhs2.hour) return false;
    if (rhs1.minute<rhs2.minute) return true;
    return false;
}
bool operator <(const time &rhs1,const time &rhs2)
{
    if (rhs1.hour<rhs2.hour) return true;
    if (rhs1.hour>rhs2.hour) return false;
    if (rhs1.minute<rhs2.minute) return true;
    return false;
}
bool check_time(date &a_date,time &a_time,date &b_date,time &b_time)
{
    if (a_date<b_date) return true;
    if (b_date<a_date) return false;
    if (b_time<a_time) return false;
    return true;
}
void add_time(date &now_date,time &now_time,int val)
{
    int tmp=val/60;
    val%=60;
    now_time.minute+=val;
    if (now_time.minute>=60)
    {
        ++tmp;
        now_time.minute-=60;
    }
    val=tmp;
    tmp=val/24;
    val%=24;
    now_time.hour+=val;
    if (now_time.hour>=24)
    {
        ++tmp;
        now_time.hour-=24;
    }
    if (tmp==0) return;
    now_date.day+=tmp;
    int k;
    if (now_date.month==7||now_date.month==8) k=31; else k=30;
    if (now_date.day>k)
    {
        now_date.day-=k;
        ++now_date.month;
    }
}
int cal_time(date &a_date,time &a_time,date &b_date,time &b_time)
{
    if (!check_time(b_date,b_time,a_date,a_time)) return 0;
    date now_date=b_date;
    time now_time=b_time;
    int res=0;
    while (now_date<a_date)
    {
        res+=24*60;
        add_time(now_date,now_time,24*60);
    }
    res+=(a_time.hour-b_time.hour)*60;
    res+=(a_time.minute-b_time.minute);
    return res;
}
struct user_id
{
    char username[23];
    user_id()
    {
        memset(username,0,sizeof(username));
    }
    bool operator <(user_id &rhs)
    {
        int len1=strlen(username),len2=strlen(rhs.username);
        for (int i=0;i<len1&&i<len2;i++)
            if (username[i]!=rhs.username[i]) return (username[i]<rhs.username[i]);
        return (len1<len2);
    }
    bool operator <(const user_id &rhs)
    {
        int len1=strlen(username),len2=strlen(rhs.username);
        for (int i=0;i<len1&&i<len2;i++)
            if (username[i]!=rhs.username[i]) return (username[i]<rhs.username[i]);
        return (len1<len2);
    }
};
bool operator <(const user_id &rhs1,user_id &rhs2)
{
    int len1=strlen(rhs1.username),len2=strlen(rhs2.username);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.username[i]!=rhs2.username[i]) return (rhs1.username[i]<rhs2.username[i]);
    return (len1<len2);
}
bool operator <(const user_id &rhs1,const user_id &rhs2)
{
    int len1=strlen(rhs1.username),len2=strlen(rhs2.username);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.username[i]!=rhs2.username[i]) return (rhs1.username[i]<rhs2.username[i]);
    return (len1<len2);
}
struct user
{
    char username[23];
    pair<int,int> password;
    char name[17];
    char mailAddr[33];
    int privilege;
    bool login_flag;
    user()
    {
        memset(username,0,sizeof(username));
        password.first=0;
        password.second=0;
        memset(name,0,sizeof(name));
        memset(mailAddr,0,sizeof(mailAddr));
        privilege=0;
        login_flag=false;
    }
    bool operator <(user &rhs)
    {
        int len1=strlen(username),len2=strlen(rhs.username);
        for (int i=0;i<len1&&i<len2;i++)
            if (username[i]!=rhs.username[i]) return (username[i]<rhs.username[i]);
        return (len1<len2);
    }
    bool operator <(const user &rhs)
    {
        int len1=strlen(username),len2=strlen(rhs.username);
        for (int i=0;i<len1&&i<len2;i++)
            if (username[i]!=rhs.username[i]) return (username[i]<rhs.username[i]);
        return (len1<len2);
    }
};
bool operator <(const user &rhs1,user &rhs2)
{
    int len1=strlen(rhs1.username),len2=strlen(rhs2.username);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.username[i]!=rhs2.username[i]) return (rhs1.username[i]<rhs2.username[i]);
    return (len1<len2);
}
bool operator <(const user &rhs1,const user &rhs2)
{
    int len1=strlen(rhs1.username),len2=strlen(rhs2.username);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.username[i]!=rhs2.username[i]) return (rhs1.username[i]<rhs2.username[i]);
    return (len1<len2);
}
struct station
{
    char station_name[33];
    station()
    {
        memset(station_name,0,sizeof(station_name));
    }
    bool operator <(station &rhs)
    {
        int len1=strlen(station_name),len2=strlen(rhs.station_name);
        for (int i=0;i<len1&&i<len2;i++)
            if (station_name[i]!=rhs.station_name[i]) return (station_name[i]<rhs.station_name[i]);
        return (len1<len2);
    }
    bool operator <(const station &rhs)
    {
        int len1=strlen(station_name),len2=strlen(rhs.station_name);
        for (int i=0;i<len1&&i<len2;i++)
            if (station_name[i]!=rhs.station_name[i]) return (station_name[i]<rhs.station_name[i]);
        return (len1<len2);
    }
};
bool operator <(const station &rhs1,station &rhs2)
{
    int len1=strlen(rhs1.station_name),len2=strlen(rhs2.station_name);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.station_name[i]!=rhs2.station_name[i]) return (rhs1.station_name[i]<rhs2.station_name[i]);
    return (len1<len2);
}
bool operator <(const station &rhs1,const station &rhs2)
{
    int len1=strlen(rhs1.station_name),len2=strlen(rhs2.station_name);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.station_name[i]!=rhs2.station_name[i]) return (rhs1.station_name[i]<rhs2.station_name[i]);
    return (len1<len2);
}
struct train_id
{
    char trainID[23];
    train_id()
    {
        memset(trainID,0,sizeof(trainID));
    }
    bool operator <(train_id &rhs)
    {
        int len1=strlen(trainID),len2=strlen(rhs.trainID);
        for (int i=0;i<len1&&i<len2;i++)
            if (trainID[i]!=rhs.trainID[i]) return (trainID[i]<rhs.trainID[i]);
        return (len1<len2);
    }
    bool operator <(const train_id &rhs)
    {
        int len1=strlen(trainID),len2=strlen(rhs.trainID);
        for (int i=0;i<len1&&i<len2;i++)
            if (trainID[i]!=rhs.trainID[i]) return (trainID[i]<rhs.trainID[i]);
        return (len1<len2);
    }
};
bool operator <(const train_id &rhs1,train_id &rhs2)
{
    int len1=strlen(rhs1.trainID),len2=strlen(rhs2.trainID);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.trainID[i]!=rhs2.trainID[i]) return (rhs1.trainID[i]<rhs2.trainID[i]);
    return (len1<len2);
}
bool operator <(const train_id &rhs1,const train_id &rhs2)
{
    int len1=strlen(rhs1.trainID),len2=strlen(rhs2.trainID);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.trainID[i]!=rhs2.trainID[i]) return (rhs1.trainID[i]<rhs2.trainID[i]);
    return (len1<len2);
}
struct train
{
    char trainID[23];
    int stationNum;
    station stations[103];
    int seatNum;
    int prices[103],sum_prices[103];
    time startTimes;
    int travelTimes[103],sum_travelTimes[103];
    int stopoverTimes[103],sum_stopoverTimes[103];
    date left_saleDate,right_saleDate;
    char type;
    int current_status;
    int ticket_num[94][101];
    train()
    {
        memset(trainID,0,sizeof(trainID));
        stationNum=0;
        seatNum=0;
        memset(prices,0,sizeof(prices));
        memset(sum_prices,0,sizeof(sum_prices));
        memset(travelTimes,0,sizeof(travelTimes));
        memset(sum_travelTimes,0,sizeof(sum_travelTimes));
        memset(stopoverTimes,0,sizeof(stopoverTimes));
        memset(sum_stopoverTimes,0,sizeof(sum_stopoverTimes));
        type='\0';
        current_status=0;
        memset(ticket_num,0,sizeof(ticket_num));
    }
    bool operator <(train &rhs)
    {
        int len1=strlen(trainID),len2=strlen(rhs.trainID);
        for (int i=0;i<len1&&i<len2;i++)
            if (trainID[i]!=rhs.trainID[i]) return (trainID[i]<rhs.trainID[i]);
        return (len1<len2);
    }
    bool operator <(const train &rhs)
    {
        int len1=strlen(trainID),len2=strlen(rhs.trainID);
        for (int i=0;i<len1&&i<len2;i++)
            if (trainID[i]!=rhs.trainID[i]) return (trainID[i]<rhs.trainID[i]);
        return (len1<len2);
    }
};
bool operator <(const train &rhs1,train &rhs2)
{
    int len1=strlen(rhs1.trainID),len2=strlen(rhs2.trainID);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.trainID[i]!=rhs2.trainID[i]) return (rhs1.trainID[i]<rhs2.trainID[i]);
    return (len1<len2);
}
bool operator <(const train &rhs1,const train &rhs2)
{
    int len1=strlen(rhs1.trainID),len2=strlen(rhs2.trainID);
    for (int i=0;i<len1&&i<len2;i++)
        if (rhs1.trainID[i]!=rhs2.trainID[i]) return (rhs1.trainID[i]<rhs2.trainID[i]);
    return (len1<len2);
}
struct ans1_order
{
    train_id ans_train_id;
    date d;
    time t;
    int pos_s,pos_t;
    int travel_time;
    int travel_cost;
    int travel_max;
    ans1_order()
    {
        pos_s=0;
        pos_t=0;
        travel_time=0;
        travel_cost=0;
        travel_max=0;
    }
};
bool cmp1_time(ans1_order &a,ans1_order &b)
{
    if (a.travel_time!=b.travel_time) return (a.travel_time<b.travel_time);
    return (a.ans_train_id<b.ans_train_id);
}
bool cmp1_cost(ans1_order &a,ans1_order &b)
{
    if (a.travel_cost!=b.travel_cost) return (a.travel_cost<b.travel_cost);
    return (a.ans_train_id<b.ans_train_id);
}
bool order1_check(ans1_order &a,ans1_order &b,bool flag)
{
    if (flag) return cmp1_time(a,b); else return cmp1_cost(a,b);
}
struct ans2_order
{
    train_id ans_train1_id,ans_train2_id;
    date d1,d2;
    time t1,t2;
    int pos1_s,pos1_t,pos2_s,pos2_t;
    int travel_time,travel1_time;
    int travel1_cost,travel2_cost;
    int travel1_max,travel2_max;
    ans2_order()
    {
        pos1_s=0;
        pos1_t=0;
        pos2_s=0;
        pos2_t=0;
        travel_time=0;
        travel1_time=0;
        travel1_cost=0;
        travel2_cost=0;
        travel1_max=0;
        travel2_max=0;
    }
};
bool cmp2_time(ans2_order &a,ans2_order &b)
{
    if (a.travel_time!=b.travel_time) return (a.travel_time<b.travel_time);
    if (a.travel1_time!=b.travel1_time) return (a.travel1_time<b.travel1_time);
    return (a.ans_train1_id<b.ans_train1_id);
}
bool cmp2_cost(ans2_order &a,ans2_order &b)
{
    if (a.travel1_cost+a.travel2_cost!=b.travel1_cost+b.travel2_cost)
        return (a.travel1_cost+a.travel2_cost<b.travel1_cost+b.travel2_cost);
    if (a.travel1_time!=b.travel1_time) return (a.travel1_time<b.travel1_time);
    return (a.ans_train1_id<b.ans_train1_id);
}
bool order2_check(ans2_order &a,ans2_order &b,bool flag)
{
    if (flag) return cmp2_time(a,b); else return cmp2_cost(a,b);
}
struct order
{
    train_id t;
    date tmp_d;
    time tmp_t;
    int idx;
    int pos_s,pos_t;
    int num,price;
    bool alter_flag;
    int status;
    order()
    {
        idx=0;
        pos_s=0;
        pos_t=0;
        num=0;
        price=0;
        alter_flag=false;
        status=0;
    }
};
bool check_new_file(const string &path)
{
    fstream test;
    bool ans;
    test.open(path,ios::in);
    if (!test.fail()) ans=false; else ans=true;
    test.close();
    return ans;
}
LRUBPTree<user_id,user> user_structure(__File1,__File2,check_new_file(__File1));
LRUBPTree<train_id,train> train_structure(__File3,__File4,check_new_file(__File3));
LRUBPTree<pair<station,train_id>,int> station_structure(__File5,__File6,check_new_file(__File5));
LRUBPTree<pair<user_id,int>,order> order_structure(__File7,__File8,check_new_file(__File7));
LRUBPTree<pair<train_id,int>,pair<user_id,order> > alter_structure(__File9,__File10,check_new_file(__File9));
void pre_work()
{
    cnt=0;
    for (int i=0;i<16;i++)
    {
        int pos=0;
        int len=strlen(instruction_name[i]);
        for (int j=0;j<len;j++)
        {
            int t=instruction_name[i][j]-90;
            if (to[pos][t]==0) to[pos][t]=++cnt;
            pos=to[pos][t];
        }
        instruction_id[pos]=i;
    }
}
void add_user()
{
    user_id cur_id,tmp_id;
    user tmp;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='c') scanf("%s",cur_id.username);
        else if (op=='u') 
        {
            scanf("%s",tmp_id.username);
            int len=strlen(tmp_id.username);
            for (int i=0;i<len;i++)
                tmp.username[i]=tmp_id.username[i];
        }
        else if (op=='p') 
        {
            char s[37];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                tmp.password.first=(307LL*tmp.password.first+s[i]+1)%modp1;
                tmp.password.second=(307LL*tmp.password.second+s[i]+1)%modp2;
            }
        }
        else if (op=='n') scanf("%s",tmp.name);
        else if (op=='m') scanf("%s",tmp.mailAddr);
        else if (op=='g') scanf("%d",&tmp.privilege);
        op=getchar();
    }
    if (user_structure.find(tmp_id)!=user_structure.end())
    {
        printf("%d\n",-1);
        return;
    }
    if (user_structure.size()!=0)
    {
        LRUBPTree<user_id,user>::iterator it=user_structure.find(cur_id);
        if ((it==user_structure.end())||(!it.data().login_flag)||(it.data().privilege<=tmp.privilege))
        {
            printf("%d\n",-1);
            return;
        }
    } else tmp.privilege=10;
    printf("%d\n",0);
    user_structure.insert(tmp_id,tmp);
}
void login()
{
    user_id cur_id;
    pair<int,int> tmp_password;
    tmp_password.first=0;
    tmp_password.second=0;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='u') scanf("%s",cur_id.username);
        else if (op=='p') 
        {
            char p[33];
            scanf("%s",p);
            int len=strlen(p);
            for (int i=0;i<len;i++)
            {
                tmp_password.first=(307LL*tmp_password.first+p[i]+1)%modp1;
                tmp_password.second=(307LL*tmp_password.second+p[i]+1)%modp2;
            }
        }
        op=getchar();
    }
    LRUBPTree<user_id,user>::iterator it=user_structure.find(cur_id);
    if (it==user_structure.end()||it.data().login_flag)
    {
        printf("%d\n",-1);
        return;
    }
    bool flag=true;
    if (tmp_password!=it.data().password) flag=false;
    if (!flag)
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    it.data(true).login_flag=true;
}
void logout()
{
    user_id cur_id;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='u') scanf("%s",cur_id.username);
        op=getchar();
    }
    LRUBPTree<user_id,user>::iterator it=user_structure.find(cur_id);
    if (it==user_structure.end()||(!it.data().login_flag))
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    it.data(true).login_flag=false;
}
void query_profile()
{
    user_id cur_id,tmp_id;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='c') scanf("%s",cur_id.username);
        else if (op=='u') scanf("%s",tmp_id.username);
        op=getchar();
    }
    LRUBPTree<user_id,user>::iterator it1=user_structure.find(cur_id);
    LRUBPTree<user_id,user>::iterator it2=user_structure.find(tmp_id);
    if (it1==user_structure.end()||it2==user_structure.end()||(!it1.data().login_flag)||(it1.data().privilege<it2.data().privilege)||((it1.data().privilege==it2.data().privilege)&&(it1!=it2)))
    {
        printf("%d\n",-1);
        return;
    }
    int len=strlen(it2.data().username);
    for (int i=0;i<len;i++)
        printf("%c",it2.data().username[i]);
    printf(" ");
    len=strlen(it2.data().name);
    for (int i=0;i<len;i++)
        printf("%c",it2.data().name[i]);
    printf(" ");
    len=strlen(it2.data().mailAddr);
    for (int i=0;i<len;i++)
        printf("%c",it2.data().mailAddr[i]);
    printf(" ");
    printf("%d\n",it2.data().privilege);
}
void modify_profile()
{
    user_id cur_id,tmp_id;
    user tmp;
    char op=getchar();
    bool flag_p=false,flag_n=false,flag_m=false,flag_g=false;
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='c') scanf("%s",cur_id.username);
        else if (op=='u') scanf("%s",tmp_id.username);
        else if (op=='p') 
        {
            char s[33];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                tmp.password.first=(307LL*tmp.password.first+s[i]+1)%modp1;
                tmp.password.second=(307LL*tmp.password.second+s[i]+1)%modp2;
            }
            flag_p=true;
        }
        else if (op=='n') 
        {
            scanf("%s",tmp.name);
            flag_n=true;
        }
        else if (op=='m') 
        {
            scanf("%s",tmp.mailAddr);
            flag_m=true;
        }
        else if (op=='g') 
        {
            scanf("%d",&tmp.privilege);
            flag_g=true;
        }
        op=getchar();
    }
    LRUBPTree<user_id,user>::iterator it1=user_structure.find(cur_id);
    LRUBPTree<user_id,user>::iterator it2=user_structure.find(tmp_id);
    if (it1==user_structure.end()||it2==user_structure.end()||(!it1.data().login_flag)||(it1.data().privilege<it2.data().privilege)||((it1.data().privilege==it2.data().privilege)&&(it1!=it2)))
    {
        printf("%d\n",-1);
        return;
    }
    if (flag_g&&(tmp.privilege>=it1.data().privilege))
    {
        printf("%d\n",-1);
        return;
    }
    int len=strlen(tmp_id.username);
    for (int i=0;i<len;i++)
        printf("%c",tmp_id.username[i]);
    printf(" ");
    if (flag_p) it2.data(true).password=tmp.password;
    if (flag_n)
    {
        memset(it2.data(true).name,0,sizeof(it2.data().name));
        len=strlen(tmp.name);
        for (int i=0;i<len;i++)
            it2.data(true).name[i]=tmp.name[i];
    }
    len=strlen(it2.data().name);
    for (int i=0;i<len;i++)
        printf("%c",it2.data().name[i]);
    printf(" ");
    if (flag_m)
    {
        memset(it2.data(true).mailAddr,0,sizeof(it2.data().mailAddr));
        len=strlen(tmp.mailAddr);
        for (int i=0;i<len;i++)
            it2.data(true).mailAddr[i]=tmp.mailAddr[i];
    }
    len=strlen(it2.data().mailAddr);
    for (int i=0;i<len;i++)
        printf("%c",it2.data().mailAddr[i]);
    printf(" ");
    if (flag_g) it2.data(true).privilege=tmp.privilege;
    printf("%d\n",it2.data().privilege);
}
void add_train()
{
    train_id tmp_id;
    train tmp;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='i') 
        {
            scanf("%s",tmp_id.trainID);
            int len=strlen(tmp_id.trainID);
            for (int i=0;i<len;i++)
                tmp.trainID[i]=tmp_id.trainID[i];
        }
        else if (op=='n') scanf("%d",&tmp.stationNum);
        else if (op=='m') scanf("%d",&tmp.seatNum);
        else if (op=='s')
        {
            int pos1=0,pos2=0;
            char s[3207];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                if (s[i]!='|') tmp.stations[pos1].station_name[pos2++]=s[i];
                else
                {
                    ++pos1;
                    pos2=0;
                }
            }
        } else if (op=='p')
        {
            int pos=0;
            char s[3207];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                if (s[i]!='|') tmp.prices[pos]=10*tmp.prices[pos]+s[i]-'0';
                else ++pos;
            }
        } else if (op=='x')
        {
            char s[13];
            scanf("%s",s);
            bool flag=false;
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                if (s[i]!=':')
                {
                    if (!flag) tmp.startTimes.hour=10*tmp.startTimes.hour+s[i]-'0';
                    else tmp.startTimes.minute=10*tmp.startTimes.minute+s[i]-'0';
                } else flag=true;
            }
        } else if (op=='t')
        {
            int pos=0;
            char s[1207];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                if (s[i]!='|') tmp.travelTimes[pos]=10*tmp.travelTimes[pos]+s[i]-'0';
                else ++pos;
            }
        } else if (op=='o')
        {
            int pos=0;
            char s[1207];
            scanf("%s",s);
            int len=strlen(s);
            for (int i=0;i<len;i++)
            {
                if (s[i]!='|') tmp.stopoverTimes[pos]=10*tmp.stopoverTimes[pos]+s[i]-'0';
                else ++pos;
            }
        } else if (op=='d')
        {
            char s[20];
            scanf("%s",s);
            int len=strlen(s);
            int pos=0;
            for (int i=0;i<len;i++)
            {
                if (s[i]!='-'&&s[i]!='|')
                {
                    if (pos==0) tmp.left_saleDate.month=10*tmp.left_saleDate.month+s[i]-'0';
                    else if (pos==1) tmp.left_saleDate.day=10*tmp.left_saleDate.day+s[i]-'0';
                    else if (pos==2) tmp.right_saleDate.month=10*tmp.right_saleDate.month+s[i]-'0';
                    else tmp.right_saleDate.day=10*tmp.right_saleDate.day+s[i]-'0';
                } else ++pos;
            }
        } else if (op=='y') 
        {
            tmp.type=getchar();
            while (tmp.type<'A'||tmp.type>'Z') tmp.type=getchar();
        }
        op=getchar();
    }
    LRUBPTree<train_id,train>::iterator it=train_structure.find(tmp_id);
    if (it!=train_structure.end()&&it.data().current_status!=-1)
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    if (it==train_structure.end())
    {
        for (int k=0;k<94;k++)
            for (int i=0;i<tmp.stationNum-1;i++)
                tmp.ticket_num[k][i]=tmp.seatNum;
        for (int i=0;i<tmp.stationNum-1;i++)
            tmp.sum_travelTimes[i+1]=tmp.sum_travelTimes[i]+tmp.travelTimes[i];
        for (int i=0;i<tmp.stationNum-2;i++)
            tmp.sum_stopoverTimes[i+1]=tmp.sum_stopoverTimes[i]+tmp.stopoverTimes[i];
        for (int i=0;i<tmp.stationNum-1;i++)
            tmp.sum_prices[i+1]=tmp.sum_prices[i]+tmp.prices[i];
        train_structure.insert(tmp_id,tmp);
    } else
    {
        it.data(true).current_status=0;
        for (int k=0;k<94;k++)
            for (int i=0;i<tmp.stationNum-1;i++)
                tmp.ticket_num[k][i]=tmp.seatNum;
        for (int i=0;i<tmp.stationNum-1;i++)
            tmp.sum_travelTimes[i+1]=tmp.sum_travelTimes[i]+tmp.travelTimes[i];
        for (int i=0;i<tmp.stationNum-2;i++)
            tmp.sum_stopoverTimes[i+1]=tmp.sum_stopoverTimes[i]+tmp.stopoverTimes[i];
        for (int i=0;i<tmp.stationNum-1;i++)
            tmp.sum_prices[i+1]=tmp.sum_prices[i]+tmp.prices[i];
        it.data(true).stationNum=tmp.stationNum;
        for (int i=0;i<tmp.stationNum;i++)
        {
            memset(it.data(true).stations[i].station_name,0,sizeof(it.data().stations[i].station_name));
            int len=strlen(tmp.stations[i].station_name);
            for (int j=0;j<len;j++)
                it.data(true).stations[i].station_name[j]=tmp.stations[i].station_name[j];
        }
        it.data(true).seatNum=tmp.seatNum;
        memset(it.data(true).prices,0,sizeof(it.data().prices));
        memset(it.data(true).sum_prices,0,sizeof(it.data().sum_prices));
        for (int i=0;i<tmp.stationNum-1;i++)
            it.data(true).prices[i]=tmp.prices[i];
        for (int i=0;i<tmp.stationNum;i++)
            it.data(true).sum_prices[i]=tmp.sum_prices[i];
        it.data(true).startTimes.hour=tmp.startTimes.hour;
        it.data(true).startTimes.minute=tmp.startTimes.minute;
        memset(it.data(true).travelTimes,0,sizeof(it.data().travelTimes));
        memset(it.data(true).sum_travelTimes,0,sizeof(it.data().sum_travelTimes));
        for (int i=0;i<tmp.stationNum-1;i++)
            it.data(true).travelTimes[i]=tmp.travelTimes[i];
        for (int i=0;i<tmp.stationNum;i++)
            it.data(true).sum_travelTimes[i]=tmp.sum_travelTimes[i];
        memset(it.data(true).stopoverTimes,0,sizeof(it.data().stopoverTimes));
        memset(it.data(true).sum_stopoverTimes,0,sizeof(it.data().sum_stopoverTimes));
        for (int i=0;i<tmp.stationNum-2;i++)
            it.data(true).stopoverTimes[i]=tmp.stopoverTimes[i];
        for (int i=0;i<tmp.stationNum-1;i++)
            it.data(true).sum_stopoverTimes[i]=tmp.sum_stopoverTimes[i];
        it.data(true).left_saleDate.month=tmp.left_saleDate.month;
        it.data(true).left_saleDate.day=tmp.left_saleDate.day;
        it.data(true).right_saleDate.month=tmp.right_saleDate.month;
        it.data(true).right_saleDate.day=tmp.right_saleDate.day;
        it.data(true).type=tmp.type;
        memset(it.data(true).ticket_num,0,sizeof(it.data().ticket_num));
        for (int k=0;k<94;k++)
            for (int i=0;i<tmp.stationNum-1;i++)
                it.data(true).ticket_num[k][i]=tmp.ticket_num[k][i];
    }
}
void release_train()
{
    train_id cur_id;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='i') scanf("%s",cur_id.trainID);
        op=getchar();
    }
    LRUBPTree<train_id,train>::iterator it=train_structure.find(cur_id);
    if (it==train_structure.end()||it.data().current_status!=0)
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    it.data(true).current_status=1;
    for (int i=0;i<(it.data().stationNum);i++)
    {
        pair<station,train_id> tmp;
        tmp.first=it.data().stations[i];
        tmp.second=it.key();
        station_structure.insert(tmp,i);
    }
}
void query_train()
{
    train_id cur_id;
    date d;
    date now_date(0,0);
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='i') scanf("%s",cur_id.trainID);
        else if (op=='d')
        {
            char s[13];
            scanf("%s",s);
            int len=strlen(s);
            bool flag=false;
            for (int i=0;i<len;i++)
            {
                if (s[i]!='-')
                {
                    if (!flag) now_date.month=10*now_date.month+s[i]-'0';
                    else now_date.day=10*now_date.day+s[i]-'0';
                } else flag=true;
            }
        }
        op=getchar();
    }
    d=now_date;
    LRUBPTree<train_id,train>::iterator it=train_structure.find(cur_id);
    if ((it==train_structure.end())||(now_date<it.data().left_saleDate)||(it.data().right_saleDate<now_date)||(it.data().current_status==-1))
    {
        printf("%d\n",-1);
        return;
    }
    int len=strlen(it.data().trainID);
    for (int i=0;i<len;i++)
        printf("%c",it.data().trainID[i]);
    printf(" ");
    printf("%c\n",it.data().type);
    time now_time=it.data().startTimes;
    int total_price=0;
    for (int i=0;i<it.data().stationNum;i++)
    {
        len=strlen(it.data().stations[i].station_name);
        for (int j=0;j<len;j++)
            printf("%c",it.data().stations[i].station_name[j]);
        printf(" ");
        if (i==0) printf("xx-xx xx:xx ");
        else
        {
            if (now_date.month<10) printf("0");
            printf("%d-",now_date.month);
            if (now_date.day<10) printf("0");
            printf("%d ",now_date.day);
            if (now_time.hour<10) printf("0");
            printf("%d:",now_time.hour);
            if (now_time.minute<10) printf("0");
            printf("%d ",now_time.minute);
            if (i!=(it.data().stationNum)-1) add_time(now_date,now_time,it.data().stopoverTimes[i-1]);
        }
        printf("-> ");
        if (i==(it.data().stationNum)-1) printf("xx-xx xx:xx ");
        else
        {
            if (now_date.month<10) printf("0");
            printf("%d-",now_date.month);
            if (now_date.day<10) printf("0");
            printf("%d ",now_date.day);
            if (now_time.hour<10) printf("0");
            printf("%d:",now_time.hour);
            if (now_time.minute<10) printf("0");
            printf("%d ",now_time.minute);
        }
        printf("%d ",total_price);
        if (i!=(it.data().stationNum)-1) 
        {
            printf("%d\n",it.data().ticket_num[calc_date(d)][i]); 
            total_price+=it.data().prices[i];
            add_time(now_date,now_time,it.data().travelTimes[i]);
        } else printf("x\n");
    }
}
void delete_train()
{
    train_id cur_id;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='i') scanf("%s",cur_id.trainID);
        op=getchar();
    }
    LRUBPTree<train_id,train>::iterator it=train_structure.find(cur_id);
    if (it==train_structure.end()||it.data().current_status!=0)
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    it.data(true).current_status=-1;
}
void query_ticket()
{
    station s,t;
    date h(0,0);
    bool q=true;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='s') scanf("%s",s.station_name);
        else if (op=='t') scanf("%s",t.station_name);
        else if (op=='d')
        {
            char s[13];
            scanf("%s",s);
            int len=strlen(s);
            bool flag=false;
            for (int i=0;i<len;i++)
            {
                if (s[i]!='-')
                {
                    if (!flag) h.month=10*h.month+s[i]-'0';
                    else h.day=10*h.day+s[i]-'0';
                } else flag=true;
            }
        } else if (op=='p')
        {
            char s[13];
            scanf("%s",s);
            if (s[0]=='t') q=true; else q=false;
        }
        op=getchar();
    }
    pair<station,train_id> cur;
    cur.first=s;
    LRUBPTree<pair<station,train_id>,int>::iterator it,end_it;
    int cnt=0;
    psgi::vector<ans1_order> ans;
    ans.clear();
    int T;
    for (it=station_structure.lower_bound(cur),end_it=station_structure.end();it!=end_it;it++)
    {
        if (s<it.key().first) break;
        train_id tmp_id=it.key().second;
        LRUBPTree<train_id,train>::iterator itt=train_structure.find(tmp_id);
        if (itt.data().current_status!=1) continue;
        int pos1=it.data(),pos2=-1;
        pair<station,train_id> tmp_pair;
        tmp_pair.first=t;
        tmp_pair.second=tmp_id;
        LRUBPTree<pair<station,train_id>,int>::iterator It=station_structure.find(tmp_pair);
        if (It!=station_structure.end()) pos2=It.data();
        if (pos1!=-1&&pos2!=-1&&pos1<pos2)
        {
            bool flag=true;
            int val=itt.data().sum_travelTimes[pos1]+itt.data().sum_stopoverTimes[pos1];
            date tmp_d=itt.data().left_saleDate;
            time tmp_t=itt.data().startTimes;
            add_time(tmp_d,tmp_t,val);
            if (h<tmp_d) flag=false;
            T=cal_time(h,tmp_t,tmp_d,tmp_t)/(24*60);
            tmp_d=itt.data().right_saleDate;
            tmp_t=itt.data().startTimes;
            add_time(tmp_d,tmp_t,val);
            if (tmp_d<h) flag=false;
            if (flag)
            {
                ++cnt;
                ans1_order Tmp;
                Tmp.ans_train_id=tmp_id;
                Tmp.d=h;
                Tmp.t=tmp_t;
                Tmp.pos_s=pos1;
                Tmp.pos_t=pos2;
                Tmp.travel_max=100000;
                int idx=calc_date(itt.data().left_saleDate)+T;
                Tmp.travel_time=itt.data().sum_travelTimes[pos2]-itt.data().sum_travelTimes[pos1];
                Tmp.travel_time+=itt.data().sum_stopoverTimes[pos2-1]-itt.data().sum_stopoverTimes[pos1];
                Tmp.travel_cost=itt.data().sum_prices[pos2]-itt.data().sum_prices[pos1];
                for (int i=pos1;i<pos2;i++)
                    if (itt.data().ticket_num[idx][i]<Tmp.travel_max) Tmp.travel_max=itt.data().ticket_num[idx][i];
                ans.push_back(Tmp);
            }
        }
    }
    if (q) sort(ans.begin(),ans.end(),cmp1_time); else sort(ans.begin(),ans.end(),cmp1_cost);
    printf("%d\n",cnt);
    for (int i=0;i<cnt;i++)
    {
        int len=strlen(ans[i].ans_train_id.trainID);
        for (int j=0;j<len;j++)
            printf("%c",ans[i].ans_train_id.trainID[j]);
        printf(" ");
        len=strlen(s.station_name);
        for (int j=0;j<len;j++)
            printf("%c",s.station_name[j]);
        printf(" ");
        if (ans[i].d.month<10) printf("0");
        printf("%d-",ans[i].d.month);
        if (ans[i].d.day<10) printf("0");
        printf("%d ",ans[i].d.day);
        if (ans[i].t.hour<10) printf("0");
        printf("%d:",ans[i].t.hour);
        if (ans[i].t.minute<10) printf("0");
        printf("%d ",ans[i].t.minute);
        printf("-> ");
        len=strlen(t.station_name);
        for (int j=0;j<len;j++)
            printf("%c",t.station_name[j]);
        printf(" ");
        date tmp_d=ans[i].d;
        time tmp_t=ans[i].t;
        add_time(tmp_d,tmp_t,ans[i].travel_time);
        if (tmp_d.month<10) printf("0");
        printf("%d-",tmp_d.month);
        if (tmp_d.day<10) printf("0");
        printf("%d ",tmp_d.day);
        if (tmp_t.hour<10) printf("0");
        printf("%d:",tmp_t.hour);
        if (tmp_t.minute<10) printf("0");
        printf("%d ",tmp_t.minute);
        printf("%d ",ans[i].travel_cost);
        printf("%d\n",ans[i].travel_max);
    }
}
void query_transfer()
{
    station s,t;
    date h(0,0);
    bool q=true;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='s') scanf("%s",s.station_name);
        else if (op=='t') scanf("%s",t.station_name);
        else if (op=='d')
        {
            char s[13];
            scanf("%s",s);
            int len=strlen(s);
            bool flag=false;
            for (int i=0;i<len;i++)
            {
                if (s[i]!='-')
                {
                    if (!flag) h.month=10*h.month+s[i]-'0';
                    else h.day=10*h.day+s[i]-'0';
                } else flag=true;
            }
        } else if (op=='p')
        {
            char s[13];
            scanf("%s",s);
            if (s[0]=='t') q=true; else q=false;
        }
        op=getchar();
    }
    LRUBPTree<pair<station,train_id>,int>::iterator it1,it2,end_it=station_structure.end();
    pair<station,train_id> cur_s,cur_t;
    cur_s.first=s;
    cur_t.first=t;
    ans2_order ans;
    bool flag_find=false;
    for (it1=station_structure.lower_bound(cur_s);(it1!=end_it)&&(!(s<it1.key().first))&&(!(it1.key().first<s));it1++)
    {
        train_id tmp1_train_id=it1.key().second;
        LRUBPTree<train_id,train>::iterator itt1=train_structure.find(tmp1_train_id);
        if (itt1.data().current_status!=1) continue;
        for (it2=station_structure.lower_bound(cur_t);(it2!=end_it)&&(!(t<it2.key().first))&&(!(it2.key().first<t));it2++)
        {
            train_id tmp2_train_id=it2.key().second;
            LRUBPTree<train_id,train>::iterator itt2=train_structure.find(tmp2_train_id);
            if (itt2.data().current_status!=1) continue;
            if ((!(itt1.data()<itt2.data()))&&(!(itt2.data()<itt1.data()))) continue;
            int pos1=-1,pos4=-1;
            for (int i=0;i<itt1.data().stationNum;i++)
                if ((!(itt1.data().stations[i]<s))&&(!(s<itt1.data().stations[i]))) 
                {
                    pos1=i;
                    break;
                }
            if (pos1==-1) continue;
            for (int i=0;i<itt2.data().stationNum;i++)
                if ((!(itt2.data().stations[i]<t))&&(!(t<itt2.data().stations[i]))) 
                {
                    pos4=i;
                    break;
                }
            if (pos4==-1) continue;
            int T1,T2;
            for (int pos2=pos1+1;pos2<itt1.data().stationNum;pos2++)
                for (int pos3=0;pos3<pos4;pos3++)
                {
                    if ((itt1.data().stations[pos2]<itt2.data().stations[pos3])||(itt2.data().stations[pos3]<itt1.data().stations[pos2])) continue;
                    ans2_order tmp_order;
                    tmp_order.ans_train1_id=tmp1_train_id;
                    tmp_order.ans_train2_id=tmp2_train_id;
                    date tmp_date;
                    time tmp_time;
                    int total_time;
                    //check pos1_left
                    tmp_date=itt1.data().left_saleDate;
                    tmp_time=itt1.data().startTimes;
                    total_time=itt1.data().sum_travelTimes[pos1]+itt1.data().sum_stopoverTimes[pos1];
                    add_time(tmp_date,tmp_time,total_time);
                    if (h<tmp_date) continue;
                    T1=cal_time(h,tmp_time,tmp_date,tmp_time)/(24*60);
                    //check pos1_right
                    tmp_date=itt1.data().right_saleDate;
                    tmp_time=itt1.data().startTimes;
                    add_time(tmp_date,tmp_time,total_time);
                    if (tmp_date<h) continue;

                    tmp_date=h;
                    tmp_order.d1=tmp_date;
                    tmp_order.t1=tmp_time;
                    total_time=itt1.data().sum_travelTimes[pos2]-itt1.data().sum_travelTimes[pos1];
                    total_time+=itt1.data().sum_stopoverTimes[pos2-1]-itt1.data().sum_stopoverTimes[pos1];
                    add_time(tmp_date,tmp_time,total_time);
                    tmp_order.travel1_time=total_time;
                    //check pos3_right
                    date Tmp_date;
                    time Tmp_time;
                    Tmp_date=itt2.data().right_saleDate;
                    Tmp_time=itt2.data().startTimes;
                    total_time=itt2.data().sum_travelTimes[pos3]+itt2.data().sum_stopoverTimes[pos3];
                    add_time(Tmp_date,Tmp_time,total_time);
                    if (!check_time(tmp_date,tmp_time,Tmp_date,Tmp_time)) continue;
                    //check pos3_left
                    Tmp_date=itt2.data().left_saleDate;
                    Tmp_time=itt2.data().startTimes;
                    add_time(Tmp_date,Tmp_time,total_time);
                    if (Tmp_time<tmp_time) 
                    {
                        add_time(tmp_date,tmp_time,24*60);
                        tmp_time=Tmp_time;
                    } else tmp_time=Tmp_time;
                    if (tmp_date<Tmp_date) tmp_date=Tmp_date;
                    T2=cal_time(tmp_date,tmp_time,Tmp_date,Tmp_time)/(24*60);
                    tmp_order.d2=tmp_date;
                    tmp_order.t2=tmp_time;
                    int tmp_total_time=itt2.data().sum_travelTimes[pos4]-itt2.data().sum_travelTimes[pos3];
                    tmp_total_time+=itt2.data().sum_stopoverTimes[pos4-1]-itt2.data().sum_stopoverTimes[pos3];
                    add_time(tmp_date,tmp_time,tmp_total_time);
                    tmp_order.pos1_s=pos1;
                    tmp_order.pos1_t=pos2;
                    tmp_order.pos2_s=pos3;
                    tmp_order.pos2_t=pos4;
                    tmp_order.travel1_max=100000;
                    int idx1=calc_date(itt1.data().left_saleDate)+T1;
                    tmp_order.travel1_cost=itt1.data().sum_prices[pos2]-itt1.data().sum_prices[pos1];
                    for (int i=pos1;i<pos2;i++)
                        if (itt1.data().ticket_num[idx1][i]<tmp_order.travel1_max) tmp_order.travel1_max=itt1.data().ticket_num[idx1][i];
                    tmp_order.travel2_cost=itt2.data().sum_prices[pos4]-itt2.data().sum_prices[pos3];
                    tmp_order.travel2_max=100000;
                    int idx2=calc_date(itt2.data().left_saleDate)+T2;
                    for (int i=pos3;i<pos4;i++)
                        if (itt2.data().ticket_num[idx2][i]<tmp_order.travel2_max) tmp_order.travel2_max=itt2.data().ticket_num[idx2][i];
                    tmp_order.travel_time=cal_time(tmp_date,tmp_time,tmp_order.d1,tmp_order.t1);
                    if (!flag_find)
                    {
                        flag_find=true;
                        ans=tmp_order;
                    } else if (order2_check(tmp_order,ans,q)) ans=tmp_order;
                }
        }
    }
    if (!flag_find) 
    {
        printf("%d\n",0);
        return;
    }
    int len;
    date tmp_d;
    LRUBPTree<train_id,train>::iterator itt1,itt2;
    itt1=train_structure.find(ans.ans_train1_id);
    itt2=train_structure.find(ans.ans_train2_id);
    time tmp_t;
    {
        len=strlen(ans.ans_train1_id.trainID);
        for (int i=0;i<len;i++)
            printf("%c",ans.ans_train1_id.trainID[i]);
        printf(" ");
        len=strlen(itt1.data().stations[ans.pos1_s].station_name);
        for (int i=0;i<len;i++)
            printf("%c",itt1.data().stations[ans.pos1_s].station_name[i]);
        printf(" ");
        if (ans.d1.month<10) printf("0");
        printf("%d-",ans.d1.month);
        if (ans.d1.day<10) printf("0");
        printf("%d ",ans.d1.day);
        if (ans.t1.hour<10) printf("0");
        printf("%d:",ans.t1.hour);
        if (ans.t1.minute<10) printf("0");
        printf("%d ",ans.t1.minute);
        printf("-> ");
        len=strlen(itt1.data().stations[ans.pos1_t].station_name);
        for (int i=0;i<len;i++)
            printf("%c",itt1.data().stations[ans.pos1_t].station_name[i]);
        printf(" ");
        tmp_d=ans.d1;
        tmp_t=ans.t1;
        int tmp_total_time=itt1.data().sum_travelTimes[ans.pos1_t]-itt1.data().sum_travelTimes[ans.pos1_s];
        tmp_total_time+=itt1.data().sum_stopoverTimes[ans.pos1_t-1]-itt1.data().sum_stopoverTimes[ans.pos1_s];
        add_time(tmp_d,tmp_t,tmp_total_time);
        if (tmp_d.month<10) printf("0");
        printf("%d-",tmp_d.month);
        if (tmp_d.day<10) printf("0");
        printf("%d ",tmp_d.day);
        if (tmp_t.hour<10) printf("0");
        printf("%d:",tmp_t.hour);
        if (tmp_t.minute<10) printf("0");
        printf("%d ",tmp_t.minute);
        printf("%d ",ans.travel1_cost);
        printf("%d\n",ans.travel1_max);
    }
    {
        len=strlen(ans.ans_train2_id.trainID);
        for (int i=0;i<len;i++)
            printf("%c",ans.ans_train2_id.trainID[i]);
        printf(" ");
        len=strlen(itt2.data().stations[ans.pos2_s].station_name);
        for (int i=0;i<len;i++)
            printf("%c",itt2.data().stations[ans.pos2_s].station_name[i]);
        printf(" ");
        if (ans.d2.month<10) printf("0");
        printf("%d-",ans.d2.month);
        if (ans.d2.day<10) printf("0");
        printf("%d ",ans.d2.day);
        if (ans.t2.hour<10) printf("0");
        printf("%d:",ans.t2.hour);
        if (ans.t2.minute<10) printf("0");
        printf("%d ",ans.t2.minute);
        printf("-> ");
        len=strlen(itt2.data().stations[ans.pos2_t].station_name);
        for (int i=0;i<len;i++)
            printf("%c",itt2.data().stations[ans.pos2_t].station_name[i]);
        printf(" ");
        tmp_d=ans.d2;
        tmp_t=ans.t2;
        int tmp_total_time=itt2.data().sum_travelTimes[ans.pos2_t]-itt2.data().sum_travelTimes[ans.pos2_s];
        tmp_total_time+=itt2.data().sum_stopoverTimes[ans.pos2_t-1]-itt2.data().sum_stopoverTimes[ans.pos2_s];
        add_time(tmp_d,tmp_t,tmp_total_time);
        if (tmp_d.month<10) printf("0");
        printf("%d-",tmp_d.month);
        if (tmp_d.day<10) printf("0");
        printf("%d ",tmp_d.day);
        if (tmp_t.hour<10) printf("0");
        printf("%d:",tmp_t.hour);
        if (tmp_t.minute<10) printf("0");
        printf("%d ",tmp_t.minute);
        printf("%d ",ans.travel2_cost);
        printf("%d\n",ans.travel2_max);
    }
}
void buy_ticket()
{
    user_id cur_id;
    train_id tmp_id;
    date h(0,0);
    station s,t;
    int num;
    bool q=false;
    char op=getchar();
    while (op!='\n')
    {
        while (op<'a'||op>'z') op=getchar();
        if (op=='u') scanf("%s",cur_id.username);
        else if (op=='i') scanf("%s",tmp_id.trainID);
        else if (op=='d')
        {
            char s[13];
            scanf("%s",s);
            int len=strlen(s);
            bool flag=false;
            for (int i=0;i<len;i++)
            {
                if (s[i]!='-')
                {
                    if (!flag) h.month=10*h.month+s[i]-'0';
                    else h.day=10*h.day+s[i]-'0';
                } else flag=true;
            }
        }
        else if (op=='n') scanf("%d",&num);
        else if (op=='f') scanf("%s",s.station_name);
        else if (op=='t') scanf("%s",t.station_name);
        else if (op=='q')
        {
            char s[13];
            scanf("%s",s);
            if (s[0]=='t') q=true;
        }
        op=getchar();
    }
    user cur;
    LRUBPTree<user_id,user>::iterator it1=user_structure.find(cur_id);
    if (it1==user_structure.end()||(!it1.data().login_flag))
    {
        printf("%d\n",-1);
        return;
    }
    cur=it1.data();
    LRUBPTree<train_id,train>::iterator it2=train_structure.find(tmp_id);
    if (it2==train_structure.end())
    {
        printf("%d\n",-1);
        return;
    }
    if (it2.data().current_status!=1)
    {
        printf("%d\n",-1);
        return;
    }
    int pos1=-1,pos2=-1;
    for (int i=0;i<it2.data().stationNum;i++)
    {
        if ((!(s<it2.data().stations[i]))&&(!(it2.data().stations[i]<s))) pos1=i;
        if ((!(t<it2.data().stations[i]))&&(!(it2.data().stations[i]<t))) pos2=i;
    }
    if (pos1==-1||pos2==-1||pos1>=pos2)
    {
        printf("%d\n",-1);
        return;
    }
    date tmp_date=it2.data().left_saleDate;
    time tmp_time=it2.data().startTimes;
    int tmp_total_time=it2.data().sum_travelTimes[pos1]+it2.data().sum_stopoverTimes[pos1];
    add_time(tmp_date,tmp_time,tmp_total_time);
    if (h<tmp_date)
    {
        printf("%d\n",-1);
        return;
    }
    int T=cal_time(h,tmp_time,tmp_date,tmp_time)/(24*60);
    tmp_date=it2.data().right_saleDate;
    tmp_time=it2.data().startTimes;
    tmp_total_time=it2.data().sum_travelTimes[pos1]+it2.data().sum_stopoverTimes[pos1];
    add_time(tmp_date,tmp_time,tmp_total_time);
    if (tmp_date<h)
    {
        printf("%d\n",-1);
        return;
    }
    if (num>it2.data().seatNum)
    {
        printf("%d\n",-1);
        return;
    }
    int tmp_idx=calc_date(it2.data().left_saleDate)+T;
    int r=100000;
    for (int i=pos1;i<pos2;i++)
        if (it2.data().ticket_num[tmp_idx][i]<r) r=it2.data().ticket_num[tmp_idx][i];
    if (r>=num)
    {
        int sum=it2.data().sum_prices[pos2]-it2.data().sum_prices[pos1];
        printf("%lld\n",1LL*sum*num);
        for (int i=pos1;i<pos2;i++)
            it2.data(true).ticket_num[tmp_idx][i]-=num;
        order tmp_order;
        tmp_order.t=tmp_id;
        tmp_order.tmp_d=h;
        tmp_order.tmp_t=tmp_time;
        tmp_order.idx=tmp_idx;
        tmp_order.pos_s=pos1;
        tmp_order.pos_t=pos2;
        tmp_order.num=num;
        tmp_order.price=sum;
        tmp_order.alter_flag=q;
        tmp_order.status=1;
        pair<user_id,int> Tmp;
        Tmp.first=cur_id;
        Tmp.second=-(order_structure.size()+1);
        order_structure.insert(Tmp,tmp_order);
    } else if (!q) printf("%d\n",-1);
    else
    {
        printf("queue\n");
        int sum=it2.data().sum_prices[pos2]-it2.data().sum_prices[pos1];
        order tmp_order;
        tmp_order.t=tmp_id;
        tmp_order.tmp_d=h;
        tmp_order.tmp_t=tmp_time;
        tmp_order.idx=tmp_idx;
        tmp_order.pos_s=pos1;
        tmp_order.pos_t=pos2;
        tmp_order.num=num;
        tmp_order.price=sum;
        tmp_order.alter_flag=q;
        tmp_order.status=0;
        pair<user_id,int> Tmp;
        Tmp.first=cur_id;
        Tmp.second=-(order_structure.size()+1);
        order_structure.insert(Tmp,tmp_order);
        pair<train_id,int> tmp1;
        tmp1.first=tmp_id;
        tmp1.second=-Tmp.second;
        pair<user_id,order> tmp2;
        tmp2.first=cur_id;
        tmp2.second=tmp_order;
        alter_structure.insert(tmp1,tmp2);
    }
}
void query_order()
{
    user_id cur_id;
    char ch=getchar();
    while (ch!='\n')
    {
        while (ch<'a'||ch>'z') ch=getchar();
        if (ch=='u') scanf("%s",cur_id.username);
        ch=getchar();
    }
    LRUBPTree<user_id,user>::iterator it=user_structure.find(cur_id);
    if (it==user_structure.end()||(!it.data().login_flag))
    {
        printf("%d\n",-1);
        return;
    }
    pair<user_id,int> tmp;
    tmp.first=cur_id;
    tmp.second=-10000000;
    int cnt=0;
    LRUBPTree<pair<user_id,int>,order>::iterator end_it=order_structure.end();
    for (LRUBPTree<pair<user_id,int>,order>::iterator it1=order_structure.lower_bound(tmp);it1!=end_it;it1++)
    {
        if ((it1.key().first<cur_id)||(cur_id<it1.key().first)) break;
        ++cnt;
    }
    printf("%d\n",cnt);
    int len;
    for (LRUBPTree<pair<user_id,int>,order>::iterator it1=order_structure.lower_bound(tmp);it1!=end_it;it1++)
    {
        if ((it1.key().first<cur_id)||(cur_id<it1.key().first)) break;
        order tmp_order=it1.data();
        if (tmp_order.status==-1) printf("[refunded] ");
        else if (tmp_order.status==0) printf("[pending] ");
        else printf("[success] ");
        len=strlen(tmp_order.t.trainID);
        for (int i=0;i<len;i++)
            printf("%c",tmp_order.t.trainID[i]);
        printf(" ");
        LRUBPTree<train_id,train>::iterator it2=train_structure.find(tmp_order.t);
        len=strlen(it2.data().stations[tmp_order.pos_s].station_name);
        for (int i=0;i<len;i++)
            printf("%c",it2.data().stations[tmp_order.pos_s].station_name[i]);
        printf(" ");
        date tmp_date=tmp_order.tmp_d;
        time tmp_time=tmp_order.tmp_t;
        if (tmp_date.month<10) printf("0");
        printf("%d-",tmp_date.month);
        if (tmp_date.day<10) printf("0");
        printf("%d ",tmp_date.day);
        if (tmp_time.hour<10) printf("0");
        printf("%d:",tmp_time.hour);
        if (tmp_time.minute<10) printf("0");
        printf("%d ",tmp_time.minute);
        printf("-> ");
        len=strlen(it2.data().stations[tmp_order.pos_t].station_name);
        for (int i=0;i<len;i++)
            printf("%c",it2.data().stations[tmp_order.pos_t].station_name[i]);
        printf(" ");
        int tmp_total_time=it2.data().sum_travelTimes[tmp_order.pos_t]-it2.data().sum_travelTimes[tmp_order.pos_s];
        tmp_total_time+=it2.data().sum_stopoverTimes[tmp_order.pos_t-1]-it2.data().sum_stopoverTimes[tmp_order.pos_s];
        add_time(tmp_date,tmp_time,tmp_total_time);
        if (tmp_date.month<10) printf("0");
        printf("%d-",tmp_date.month);
        if (tmp_date.day<10) printf("0");
        printf("%d ",tmp_date.day);
        if (tmp_time.hour<10) printf("0");
        printf("%d:",tmp_time.hour);
        if (tmp_time.minute<10) printf("0");
        printf("%d ",tmp_time.minute);
        printf("%d %d\n",tmp_order.price,tmp_order.num);
    }
}
void refund_ticket()
{
    user_id cur_id;
    int pos;
    char ch=getchar();
    while (ch!='\n')
    {
        while (ch<'a'||ch>'z') ch=getchar();
        if (ch=='u') scanf("%s",cur_id.username);
        else if (ch=='n') scanf("%d",&pos);
        ch=getchar();
    }
    LRUBPTree<user_id,user>::iterator it=user_structure.find(cur_id);
    if ((it==user_structure.end())||(!(it.data().login_flag)))
    {
        printf("%d\n",-1);
        return;
    }
    pair<user_id,int> tmp;
    tmp.first=cur_id;
    tmp.second=-10000000;
    LRUBPTree<pair<user_id,int>,order>::iterator ans_it,end_it=order_structure.end();
    for (LRUBPTree<pair<user_id,int>,order>::iterator it1=order_structure.lower_bound(tmp);it1!=end_it;it1++)
    {
        if ((it1.key().first<cur_id)||(cur_id<it1.key().first)) break;
        --pos;
        if (pos==0)
        {
            ans_it=it1;
            break;
        }
    }
    if (pos!=0)
    {
        printf("%d\n",-1);
        return;
    }
    if (ans_it.data().status==-1)
    {
        printf("%d\n",-1);
        return;
    }
    printf("%d\n",0);
    int op=ans_it.data().status;
    ans_it.data(true).status=-1;
    LRUBPTree<train_id,train>::iterator train_it=train_structure.find(ans_it.data().t);
    if (op==1)
    {
        for (int i=ans_it.data().pos_s;i<ans_it.data().pos_t;i++)
            train_it.data(true).ticket_num[ans_it.data().idx][i]+=ans_it.data().num;
    }
    pair<train_id,int> tmp1;
    tmp1.first=ans_it.data().t;
    tmp1.second=-ans_it.key().second;
    LRUBPTree<pair<train_id,int>,pair<user_id,order> >::iterator tmp_it=alter_structure.find(tmp1),end_it1=alter_structure.end();
    if (tmp_it!=alter_structure.end()) tmp_it.data(true).second.status=-1;
    if (op!=1) return;
    tmp1.second=-10000000;
    for (LRUBPTree<pair<train_id,int>,pair<user_id,order> >::iterator alter_it=alter_structure.lower_bound(tmp1);alter_it!=end_it1;alter_it++)
    {
        if ((alter_it.key().first<tmp1.first)||(tmp1.first<alter_it.key().first)) break;
        pair<user_id,order> now=alter_it.data();
        if (now.second.idx!=ans_it.data().idx) continue;
        if (now.second.status!=0) continue;
        bool flag=true;
        for (int i=now.second.pos_s;i<now.second.pos_t;i++)
            if (train_it.data().ticket_num[now.second.idx][i]<now.second.num) flag=false;
        if (flag)
        {
            for (int i=now.second.pos_s;i<now.second.pos_t;i++)
                train_it.data(true).ticket_num[now.second.idx][i]-=now.second.num;
            alter_it.data(true).second.status=1;
            pair<user_id,int> q;
            q.first=now.first;
            q.second=-(alter_it.key().second);
            LRUBPTree<pair<user_id,int>,order>::iterator itt=order_structure.find(q);
            itt.data(true).status=1;
        }
    }
}
void clean()
{
    user_structure.clean();
    train_structure.clean();
    station_structure.clean();
    order_structure.clean();
    alter_structure.clean();
    printf("%d\n",0);
}
int main()
{
    pre_work();
    while (1)
    {
        char op[17];
        int op_len,pos=0;
        scanf("%s",op);
        op_len=strlen(op);
        for (int i=0;i<op_len;i++)
            pos=to[pos][op[i]-90];
        int type=instruction_id[pos];
        switch(type)
        {
            case 0:add_user();break;
            case 1:login();break;
            case 2:logout();break;
            case 3:query_profile();break;
            case 4:modify_profile();break;
            case 5:add_train();break;
            case 6:release_train();break;
            case 7:query_train();break;
            case 8:delete_train();break;
            case 9:query_ticket();break;
            case 10:query_transfer();break;
            case 11:buy_ticket();break;
            case 12:query_order();break;
            case 13:refund_ticket();break;
            case 14:clean();break;
            case 15:
            {
                for (LRUBPTree<user_id,user>::iterator it=user_structure.begin();it!=user_structure.end();it++)
                    it.data(true).login_flag=false;
                printf("bye\n");
                return 0;
            }
        }
    }
    return 0;
}
