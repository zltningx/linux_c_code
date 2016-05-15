#!/usr/bin/env python
# coding=utf-8

""" Created By LiT0
2016 年 5 月 8 日

Name: 进程调度
"""

from collections import OrderedDict,deque
import argparse


class ProList(object):
    """ 链表轮子： 虽然很不pythonic但是还是要写。。。
    在本实验中的作用在于存储。。。就当练手了 
    """
    class _PCB(object):
        """ 内部节点类 """
        __slot__ = 'name', 'arr_time', 'bur_time', 'beg_time' 'fin_time', 'run_time', 'wet_time' 'next'
        def __init__(self, name, arr_time=-1, bur_time=-1, prior=None, next=None):
            self.name = name
            self.arr_time = arr_time
            self.bur_time = bur_time
            self.prior = prior
            self.next = next

        def calculate(self):
            """ 计算周转时间和带权周转时间 """
            self.run_time = self.fin_time - self.arr_time
            self.wet_time = self.run_time / self.bur_time

        def __repr__(self):
            return self.name

    def __init__(self, name):
        self.head = self._PCB(name)
    
    def append(self, name, arr_time, bur_time):
        """ 添加一个节点 """

        new = self._PCB(name, arr_time, bur_time)
        temp = self.head
        while temp.next != None:
            temp = temp.next

        temp.next = new
        new.prior = temp

    def append_node(self, process):
        """ 添加一个完整节点 """
        temp = self.head
        while temp.next != None:
            temp = temp.next

        temp.next = process
        process.prior = temp
        process.next = None

    def delete(self, process):
        """ 删除一个节点 """
        pr = process.prior
        ne = process.next
        pr.next = ne
        if ne != None:
            ne.prior = pr

    def __repr__(self):
        return "LinkList of {}".format(self.head.name)


class Algorithms(object):
    """ 各个队列的初始化
    以及各种调度算法
    1, FCFS 
    """
    def __init__(self):
        self.ready = ProList('ready')
        # self.finished = ProList('finished')
        self._read_file()

    def _read_file(self):
        """ 从文件读初始数据给ready 队列 """
        with open('data.db', 'r') as f:
            arr_t = f.readline()
            bur_t = f.readline()

        arr_t = arr_t.split()
        bur_t = bur_t.split()
        for i in range(len(arr_t)):
            self.ready.append(str(i), int(arr_t[i]), int(bur_t[i]))
            print ("[+]get new process {}, start: {} surver: {}"
                   .format(str(i), arr_t[i], bur_t[i]))

    def write_file(self):
        """ 计算周转时间和带权周转时间 
        并将信息写入文件
        """
        print (Bcolors.OKGREEN + "[*] Start write to file")
        ave_time = 0
        wei_ave_time = 0
        count = 0
        temp = self.ready.head
        aver = lambda x, y: x / y  # 匿名函数 x / y
        arr_buff = ""
        bur_buff = ""
        beg_buff = ""
        fin_buff = ""
        run_buff = ""
        wet_buff = ""

        while temp.next != None:
            temp = temp.next
            arr_buff += "{:<10}".format(temp.arr_time)
            bur_buff += "{:<10}".format(temp.bur_time)
            beg_buff += "{:<10}".format(temp.beg_time)
            fin_buff += "{:<10}".format(temp.fin_time)
            run_buff += "{:<10}".format(temp.run_time)
            wet_buff += "{:<10.1f}".format(temp.wet_time)
            ave_time += temp.run_time
            wei_ave_time += temp.wet_time
            count += 1
        try:
            with open('result.db', 'w') as f:
                f.write("到达时间：         " + arr_buff + '\n')
                f.write("服务时间：         " + bur_buff + '\n')
                f.write("开始时间：         " + beg_buff + '\n')
                f.write("结束时间：         " + fin_buff + '\n')
                f.write("周转时间：         " + run_buff + '\n')
                f.write("带权周转时间：     " + wet_buff + '\n')
                f.write("平均周转时间：     {:.1f}\n".format(aver(ave_time, count)))
                f.write("平均带权周转时间： {:.1f}\n".format(aver(wei_ave_time, count)))
            print (Bcolors.BOLD + Bcolors.OKGREEN + "[+] Write to file sucessfully")
        except Exception as e:
            print (e)
            print (Bcolors.OKBLUE + '[-] Fail write to file')

    def fcfsAlgorithms(self):
        """ FCFS 先来先服务算法的实现 """
        temp = self.ready.head

        if temp is None:
            raise "Error: Empty Job List!"

        if temp.next != None:
            systime = temp.next.arr_time  # systime为当前系统时间，初始化第一个作业到达时间
        while temp.next != None:
            temp = temp.next
            temp.beg_time = systime  # 开始时间为当前系统时间systime
            systime += temp.bur_time  # 此时systime是做完一个进程之后的时间,正好是结束时间
            temp.fin_time = systime
            temp.calculate()  # 计算周转时间和带权周转时间 

    def sjfAlgorithms(self):
        """ SJF 短作业优先算法的实现 """
        temp = self.ready.head
        sjfList = OrderedDict()
        count = 0
        systime = 0

        if temp is None:
            raise "Error: Empty Job List!"

        while temp.next != None:
            count += 1
            temp = temp.next
            sjfList[temp] = temp.bur_time

        flag = True
        for i in range(count):
            min_p = min(sjfList, key=lambda k: sjfList[k])
            if flag:
                systime = min_p.arr_time
                flag = False
            if min_p.arr_time > systime:
                systime = min_p.arr_time  # 最小作业但是系统时间为到作业到达时间 将作业到达时间赋给systime
            min_p.beg_time = systime  # 开始时间为当前系统时间systime
            systime += min_p.bur_time  # 此时systime是做完一个进程之后的时间,正好是结束时间
            min_p.fin_time = systime
            min_p.calculate()  # 计算周转时间和带权周转时间 
            sjfList.pop(min_p)

    def hrrnAlgorithms(self):
        """ HRRN 高响应比优先算法的实现 """
        temp = self.ready.head
        hrrnList = list()
        systime = 0

        if temp is None:
            raise "Error: Empty Job List!"

        while temp.next != None:
            temp = temp.next
            hrrnList.append(temp)

        while len(hrrnList) != 0:
            listen = 0  # 监听：若当前系统时间小于当前存在的任一作业到达时间
            priority = 0

            for i in hrrnList:
                if i.arr_time <= systime:  # 过滤目前未到达作业
                    listen = 1  # 捕捉 监听
                    i_priority = (systime - i.arr_time + i.bur_time) / i.bur_time  # 响应比计算优先权
                    if i_priority > priority:
                        priority = i_priority
                        job_priority = i

            if listen == 0:
                systime += 1
                continue

            job_priority.beg_time = systime  # 时间计算
            systime += job_priority.bur_time
            job_priority.fin_time = systime
            job_priority.calculate()  # 计算周转时间和带权周转时间 
            hrrnList.remove(job_priority)

    def rrAlgorithms(self):
        """ RR 时间轮转算法的实现 """
        temp = self.ready.head
        rrList = OrderedDict()
        systime = 0
        q = 1   # 轮转时间片大小

        if temp is None:
            raise "Error: Empty Job List!"

        while temp.next != None:
            temp = temp.next
            t = list()
            t.append(temp.arr_time)
            t.append(temp.bur_time)
            t.append(1)  # flag 用于记录第一次作业开始时间
            rrList[temp] = t

        while len(rrList) != 0:
            for i in rrList:
                if rrList[i][0] <= systime:
                    print ("{}: {}".format(i, rrList[i][1]))
                    if rrList[i][2] == 1:
                        i.beg_time = systime  # 作业开始时间
                        rrList[i][2] = 0
                    rrList[i][1] -= q
                    if rrList[i][1] == 0:
                        print ("{} is over".format(i))
                        i.fin_time = systime  # 结束时间
                        i.calculate()  # 计算周转时间和带权周转时间 
                        del rrList[i]
                systime += q
            print ('------------')

    def mfqAlgorithms(self):
        """ MFQ 多级反馈队列调度算法的实现 """
        temp = self.ready.head
        mfqList = OrderedDict()
        fir_mfq = deque()
        sec_mfq = deque()
        thr_mfq = deque()

        f_time = 1
        s_time = 2
        t_time = 4

        systime = 0

        while temp.next != None:
            temp = temp.next
            t = list()
            t.append(temp.bur_time)
            t.append(1)
            mfqList[temp] = t

        while len(mfqList) != 0:
            for i in mfqList:  # 遍历就绪队列job 将每一次systime 更新后 到达时间到达的job加入 反馈队列
                if i.arr_time <= systime and mfqList[i][1] == 1:
                    fir_mfq.append(i)
                    mfqList[i][1] = 0
                    print ("process {} join the queue！".format(i))

            #三级反馈队列
            if len(fir_mfq) != 0:
                mfqList[fir_mfq[0]][0] -= f_time
                fir_mfq[0].beg_time = systime   # 每个job 开始运行的时间
                systime += f_time
                if mfqList[fir_mfq[0]][0] > 0:
                    sec_mfq.append(fir_mfq.popleft())
                else:
                    end = fir_mfq.popleft() # 结束，则出队
                    # 假如给了2个片 只做了一个片 则记录结束时间只加一个片(减去片后为负数) 
                    end.fin_time = systime + mfqList[end][0]  
                    end.calculate()  # 计算周转时间和带权周转时间 
                    print ("第一级队列中： process {} end running!".format(end))
                    del mfqList[end]
            elif len(sec_mfq) != 0:
                mfqList[sec_mfq[0]][0] -= s_time
                systime += s_time
                if mfqList[sec_mfq[0]][0] > 0:
                    thr_mfq.append(sec_mfq.popleft())
                else:
                    end = sec_mfq.popleft()
                    end.fin_time = systime + mfqList[end][0]
                    end.calculate()  
                    print ("第二级队列中： process {} end running!".format(end))
                    del mfqList[end]
            elif len(thr_mfq) != 0:
                mfqList[thr_mfq[0]][0] -= t_time
                systime += t_time
                if mfqList[thr_mfq[0]][0] > 0:
                    thr_mfq.append(thr_mfq.popleft())  # 最后一级队列,若未做完则加入自己队尾
                else:
                    end = thr_mfq.popleft()
                    end.fin_time = systime + mfqList[end][0]
                    end.calculate() 
                    print ("第三级队列中： process {} end running!".format(end))
                    del mfqList[end]
            else:
                systime += 1  # 三级反馈队列为空 并且 队列中还有job 则系统时间自己动！


class BankerAlgorithm(object):
    """ 银行家算法避免死锁 """

    class _PRO(object):
        """ 内部节点类 用于实现进程的独立 """
        __slot__ = 'name', 'maxx', 'allocation', 'need', 'flag'
        def __init__(self, name, maxx, allocation, need, flag=False):
            self.name = name
            self.maxx = maxx
            self.allocation = allocation
            self.need = need
            self.flag = flag

        def __repr__(self):
            return self.name

    def __init__(self):
        """ 从文件读进程初始的MAX 
        以及Allocation和Available 
        加入进程中！
        """
        self.proList = list()
        self.available = list()
        process = list()

        with open("banker.db", "r") as f:
            for i in f:
                if i.startswith('P'):
                    i = i.split()
                    process.append(i)

        tmp = process[0][-3:]
        for i in tmp:
            self.available.append(int(i))

        for p in process:
            maxx = p[1:4]
            allocation = p[4:7]
            need = self.calculate_need(maxx, allocation)
            new = self._PRO(p[0], maxx, allocation, need)
            self.proList.append(new)

    def calculate_need(self, maxx, allocation):
        """ 计算出need 并且将 maxx 和 allocation 变成 int """
        need = list()
        for i in range(len(maxx)):
            maxx[i] = int(maxx[i])
            allocation[i] = int(allocation[i])
            need.append(maxx[i] - allocation[i])
        return need

    def _is_valued(self, source, objective):
        """ 检查source 向量是否 小于等于 objective变量 """
        for i in range(len(source)):
            if source[i] > objective[i]:
                return False
        return True

    def _add(self, source, objective):
        """ 用于求source向量与objective向量的差 并保存与source中"""
        for i in range(len(source)):
            source[i] = source[i] + objective[i]

    def _sub(self, source, objective):
        """ 用于求source向量与objective向量的差 并保存与source中"""
        for i in range(len(source)):
            source[i] = source[i] - objective[i]

    def is_safe(self):
        """ 检查安全性 """
        work = self.available[:]
        count = 0

        while count != len(self.proList):
            flag = False
            for p in self.proList:
                if p.flag is False and self._is_valued(p.need, work):  # p未被访问并且可以被使用
                    flag = True
                    self._add(work, p.allocation)
                    p.flag = True
                    count += 1
                    print (p)
                    print ("%s work_set" % work)
            if not flag:  # 若找不到可以被使用的进程则返回False
                return False
        return True

    def show(self):
        """ 打印可用资源和进程状态 """
        print ("可用资源： {}".format(self.available))
        print ("Name   MAX    Allocation    Need")
        for p in self.proList:
            print ("{}: {}  {}  {}".format(p, p.maxx, p.allocation, p.need))


    def do_request(self, process, request):
        """ 尝试Get 资源 """
        p = int(list(process)[1])
        if p > (len(self.proList) - 1):  # p 程序号不大于已有程序号 issus: 没有对负数 字符型数过滤
            return False
        elif not self._is_valued(request, self.proList[p].need):  # 请求资源不大于程序需要的
            print ("[-] Error: apply too many source")
            return False
        elif not self._is_valued(request, self.available):  # 请求资源不大于目前空闲的资源
            print ("[-] Error: apply too many source")
            return False
        # 尝试将资源分配给进程p
        self._sub(self.available, request)
        self._add(self.proList[p].allocation, request)
        self._sub(self.proList[p].need, request)
        if self.is_safe():
            print (Bcolors.OKGREEN + "[+] Sucess devise request to {}".format(process))
        else:
            # 不安全，复原
            self._add(self.available, request)
            self._sub(self.proList[p].allocation, request)
            self._add(self.proList[p].need, request)
            print (Bcolors.WARNING + "[*] Warrning it's Unsafe operator")
            print (Bcolors.OKBLUE + "[-] Stop devise")


class Bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    DEEPGREEN = '\033[96m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def main():
    if args.banker:
        banker = BankerAlgorithm()
        cmd = input(Bcolors.DEEPGREEN + "Banker's Glory Shell" + Bcolors.ENDC + ">>>")
        cmd_list = ['exit', 'request', 'safe_now', 'show']
        while cmd != cmd_list[0]:
            cmd = cmd.split()
            if cmd[0] not in cmd_list:
                print (Bcolors.FAIL + "[-] Error: Command nofound")
            elif cmd[0] == cmd_list[1]:
                cmd = cmd[1:]
                request = list()
                for i in cmd[1:]:
                    request.append(int(i))
                banker.do_request(cmd[0], request)
            elif cmd[0] == cmd_list[2]:
                banker.is_safe()
            elif cmd[0] == cmd_list[3]:
                banker.show()
            cmd = input(Bcolors.DEEPGREEN + "Banker's Glory Shell" + Bcolors.ENDC + ">>>")
    job = Algorithms()
    if args.fcfs:
        job.fcfsAlgorithms()
        job.write_file()
    if args.sjf:
        job.sjfAlgorithms()
        job.write_file()
    if args.hrrn:
        job.hrrnAlgorithms()
        job.write_file()
    if args.rr:
        job.rrAlgorithms()
        job.write_file()
    if args.mfq:
        job.mfqAlgorithms()
        job.write_file()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='myprogram', description='调度算法与银行家算法')
    parser.add_argument('-v', '--version', action='version', version='%(prog)s v1.0')
    job = parser.add_mutually_exclusive_group()  # 添加一个互斥组不能同时执行同种算法
    job.add_argument('-f', '--fcfs', dest='fcfs', action='store_true', help='使用FCFS 先来先服务算法')
    job.add_argument('-s', '--sjf', dest='sjf', action='store_true', help='使用SJF 短作业优先算法')
    job.add_argument('-hr', '--hrrn', dest='hrrn', action='store_true', help='使用HRRN 高响应比优先算法')
    job.add_argument('-r', '--rr', dest='rr', action='store_true', help='使用RR 时间片轮转算法')
    job.add_argument('-m', '--mfq', dest='mfq', action='store_true', help='使用MFQ 多级反馈队列调度算法')
    parser.add_argument('--banker-shell', dest='banker', action='store_true', help='开启银行家调度算法')
    args = parser.parse_args()

    main()
