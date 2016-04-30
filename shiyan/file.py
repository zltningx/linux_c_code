#!/usr/bin/env python
# coding=utf-8

import re
import os
from datetime import datetime
from collections import deque

BLOCK_NUM = 10000

def count_inode(num):
    if num <= 10:
        return 0
    elif num > 10 and num < 16:
        return 1
    elif num > 15 and num < 41:
        n = (num - 16) // 5
        if n < 0:
            n = 0
        return n + 3
    elif num > 40:
        count = 7 + 3
        n = (num - 41) // 5
        if n < 0:
            n = 0
        if n % 5 == 0 and n != 0:
            count += 1
        return n + count

class SuperBlock:
    __slots__ = 'sup', 'block_num'
    def __init__(self, block):
        '''
        block 是 Block 类的列表
        sup 是 超级块栈 存10个块 
        '''
        self.sup = block
        self.block_num = 10

    def get_block(self):
        if self.block_num != 1:
            self.block_num -= 1
            return self.sup.pop()
        elif self.block_num == 1:
            print ('test')
            temp = self.sup.pop()
            self.sup = temp.next_group
            self.block_num = 10
            #这一行是把最后一个节点的下一个节点指向空
            #temp.next_group = None
            return temp

    def re_block(self, ire):
        if self.block_num == 10:
            self.block_num = 1
            ire.next_group = self.sup

            self.sup = list()
            self.sup.append(ire)
        elif self.block_num < 10:
            self.block_num += 1
            self.sup.append(ire)

    def show_super(self):
        print (Bcolors.BOLD + "Super : {}".format(self.block_num) + Bcolors.ENDC)
        for i in range(10):
            try:
                print ("| {} |                  | {} |"\
                       .format(self.sup[i].addr, self.sup[0].next_group[i].addr))
            except:
                try:
                    print ("| {} |                  | {} |"\
                           .format(" " * len(str(self.sup[0].addr)), self.sup[0].next_group[i].addr))
                except:
                    print ("| {} |                  | {} |"\
                           .format(" " * len(str(self.sup[0].addr))),
                           " "*len(str(self.sup[0].next_group[0].addr)))

class BlockManage(SuperBlock):
    class _Block:
        __slots__ = 'addr', 'next_group'
        def __init__(self, addr, next=None):
            self.addr = addr
            self.next_group = next
            
    def __init__(self):
        self.block_list = list()
        for i in range(BLOCK_NUM - 1, -1, -1):
            if (i + 1) % 10 == 0 and (i + 1) < BLOCK_NUM:
                temp = self._Block(i, self.block_list)
                self.block_list = list()
                self.block_list.append(temp)
            else:
                self.block_list.append(self._Block(i))
        super().__init__(self.block_list)

    def get_blocks(self, size):
        blocks = list()
        for i in range(size):
            blocks.append(self.get_block())

        return blocks

    def re_blocks(self, blocks):
        for i in range(len(blocks)):
            self.re_block(blocks.pop())

        '''
        for block in blocks:
            self.re_block(block)
        '''

class Inode:
    __slots__ = 'length', 'block_list','node'
    def __init__(self, length, block_list):
        self.length = length
        self.block_list= block_list
        self.node = deque()
        for i in range(count_inode(length)):
            self.node.append(block_list.pop())

    def ret_block(self):
        for i in self.node:
            self.block_list.append(i)
        return self.block_list

    def show_block(self):
        t = 0
        temp = self.block_list
        for i in range(len(temp)):
            n = count_inode(i+1)
            if t != n:
                if n - t > 1:
                    for h in range(t, n):
                        print (Bcolors.BOLD + Bcolors.OKGREEN\
                               + "Index Node: {}".format(self.node[h].addr) + Bcolors.ENDC)
                else:
                    print (Bcolors.BOLD + Bcolors.OKGREEN\
                           + "Index Node: {}".format(self.node[n-1].addr) + Bcolors.ENDC)
                t = n
            if i > 9 and i <= 14:
                print (Bcolors.BOLD + "\t{}".format(temp[i].addr))
            elif i > 14 and i <= 39:
                print (Bcolors.BOLD + "\t\t{}".format(temp[i].addr))
            elif i > 39:
                print (Bcolors.BOLD + "\t\t\t{}".format(temp[i].addr))
            else:
                print (Bcolors.BOLD + "{}".format(temp[i].addr))


    def show_blocks(self):
        l = list()
        cout = 0
        temp = self.block_list
        num = len(temp)
        print (Bcolors.BOLD+"inode infomation\ntotal block: {} ".format(num)+Bcolors.ENDC)
        for i in range(num):
            l.append(temp[i].addr)
            if i <= 9:
                print ("\t| {} |".format(temp[i].addr))
            if i == 9:
                print (Bcolors.BOLD + "Single List: " + Bcolors.ENDC)
                l = list()
            if i <= 12 and i > 9:
                print ("  index node: {}".format(self.node.popleft().addr), end='')
                print ("\t\t| {} |".format(temp[i].addr))
            if i == 12:
                print (Bcolors.BOLD + "Double List:" + Bcolors.ENDC)
                l = list()
            if i <= 22 and i > 12:
                if len(l) == 3:
                    for a in l:
                        print ("\t\t\t| {} |".format(a))
                    print ("\t\t\t------")
                    l = list()
                if i + 2 >num:
                    for a in l:
                        print ("\t\t\t| {} |".format(a))
                    print ("\t\t\t------")
                    l = list()
            if i == 22:
                print (Bcolors.BOLD + "Triple List" + Bcolors.ENDC)
            if i > 22:
                if len(l) == 3:
                    cout += 1
                    for a in l:
                        print ("\t\t\t\t| {} |".format(a))
                    print ("\t\t\t\t------")
                    l = list()
                if i + 2 >num:
                    for a in l:
                        print ("\t\t\t\t| {} |".format(a))
                    print ("\t\t\t\t------")
                    l = list()
                if cout == 3:
                    print ('\n')
                    cout = 0

class Linkedfile:
    class _Node:
        __slots__ = 'name', 'size', 'type', 'inode', 'datetime', 'prior', 'next', 'dir'
        def __init__(self, name, size, type, prior, next, dir):
            self.name = name
            self.size = size
            self.inode = None
            self.type = type
            self.datetime = datetime.now().strftime('%c')
            self.prior = prior
            self.next = next
            self.dir = dir

        def set_inode(self, block_list):
            self.inode = Inode(self.size, block_list)

    def __init__(self):
        self.head = self._Node("root", 0, -1, self, None, self)

        self.dian = self._Node(".", 0, 2, self.head, None, self)
        self.diandian = self._Node("..", 0, 2, self.dian, None, self)
        self.dian.next = self.diandian

        self.head.next = self.dian
        self.prior = None

    def is_exist(self, name):
        #查重
        temp = self.head
        while temp.next is not None:
            temp = temp.next
            if temp.name == name:
                print (Bcolors.FAIL + "Error: file or dirent exist")
                return True
        return False

    def find_file(self, name):
        temp = self.head
        while temp.next is not None:
            temp = temp.next
            if temp.name == name:
                return temp
        return False

    def append(self, block_list, name, size, type = 1, dir=None):
        #尾插法
        temp = self.head
        if dir == None:
            new = self._Node(name, size, type, None, None, self)
        else:
            new = self._Node(name, size, type, None, None, dir)
        new.set_inode(block_list)

        while temp.next is not None:
            temp = temp.next
        new.prior = temp
        temp.next = new
        new.next = None

    def delete_node(self, name, type=1):
        temp = self.head
        while temp.next is not None:
            temp = temp.next
            if temp.name == name:
                if type == 1:
                    if temp.type == 2:
                        print (Bcolors.FAIL + "Error: {} is not a file. omit it".format(name))
                        return True, None
                if type == 2:
                    if temp.type == 2 and temp.dir.diandian.next is not None:
                        print (Bcolors.FAIL + "Error: {} is not a empty file. omit it".format(name))
                        return True, None
                block_list = temp.inode.ret_block()
                pre = temp.prior
                succ = temp.next
                pre.next = succ
                if succ is not None:
                    succ.prior = pre
                return True, block_list
        return False, None

    def show_list(self):
        temp = self.head
        while temp.next != None:
            temp = temp.next
            if not temp.name.startswith("."):
                if temp.type == 1:
                    print ("- {:<5d} {} {}".format(temp.size, temp.datetime, temp.name))
                if temp.type == 2:
                    print ("d {:<5d} {} ".format(temp.size, temp.datetime), end='')
                    print (Bcolors.OKBLUE + "{}/".format(temp.name) + Bcolors.ENDC)

    def show_all(self):
        temp = self.head
        while temp.next != None:
            temp = temp.next
            if temp.type == 1:
                print ("- {:<5d} {} {}".format(temp.size, temp.datetime, temp.name))
            if temp.type == 2:
                    print ("d {:<5d} {} ".format(temp.size, temp.datetime), end='')
                    print (Bcolors.OKBLUE + "{}/".format(temp.name) + Bcolors.ENDC)

class Os:
    def __init__(self):
        self.root = Linkedfile()
        self.workpath = self.root
        self.block = BlockManage()

    def get_path(self):
        dir = self.workpath
        workpath = dir.head.name 
        while dir.diandian.dir.head.name != 'root':
            dir = dir.diandian.dir
            workpath = dir.head.name + "/" +workpath
        if workpath == 'root':
            return ''
        else:
            return workpath

    def cd(self, opt):
        if len(opt) < 2:
            print(Bcolors.FAIL + "Error: not enough option")
            print(Bcolors.HEADER + "Useage:\n\tcd [dir name]")
            return 
        if opt[1] == '/':
            self.workpath = self.root
            return
        flag = 0
        cmd = opt[1].split('/')
        for  c in cmd:
            temp = self.workpath.head
            while temp.next is not None:
                temp = temp.next
                if temp.name == c:
                    flag = 1
                    if temp.type != 2:
                        return Bcolors.FAIL + "Error: {} is not dirent".format(c)
                    self.workpath = temp.dir
            if not flag:
                print (Bcolors.FAIL + "Error: {} not exist".format(c))

    def ls(self,opt):
        if len(opt) < 2:
            self.workpath.show_list()
        elif opt[1] == '-a':
            self.workpath.show_all()
        else:
            print(Bcolors.FAIL + "Error: Bad option")
            print(Bcolors.HEADER + "Useage:\n\tls to show file&dir. \n\tls [-a] to show all following file&dir.")

    def touch(self, opt):
        if len(opt) > 2:
            if int(opt[2]) > BLOCK_NUM:
                print(Bcolors.FAIL + "Error: not enough disk")
                return 
            if not self.workpath.is_exist(opt[1]):
                block_list = self.block.get_blocks(int(opt[2]) + count_inode(int(opt[2])))
                self.workpath.append(block_list, opt[1], int(opt[2]))
        else:
            print(Bcolors.FAIL + "Error: not enough option")
            print(Bcolors.HEADER + "Useage:\n\ttouch [name] [size]")

    def mkdir(self, opt):
        if len(opt) > 1:
            opt = opt[1:]
            for name in opt:
                dir = Linkedfile()
                dir.head.name = name
                dir.diandian.dir = self.workpath
                if not self.workpath.is_exist(name):
                    block_list = self.block.get_blocks(1)
                    self.workpath.append(block_list, name, 1, 2, dir)
        else:
            print (Bcolors.FAIL + "Error: not enough option")
            print (Bcolors.HEADER + "Useage:\n\tmkdir [dir_name] ...")

    def rm(self,opt):
        if len(opt) < 2:
            print(Bcolors.FAIL + "Error: not enough option")
            print(Bcolors.HEADER + "Useage:\n\trm [name] ...\n\trm -r [dir_name] ...")
        elif opt[1] == '-r':
            #Bug for 删除不空文件还没有递归归还inode
            #通过 ban delete 不用文件完美解决!
            opt = opt[2:]
            current_name = self.get_current_name()
            new = list()
            for o in opt:
                if o[0] == '*':
                    o = '.' + o
                for c in current_name:
                    try:
                        optn = re.match(o, c)
                        if optn is not None and optn.group() == optn.string:
                            new.append(optn.group())
                    except:
                        continue
            if len(new) != 0:
                opt = list()
                opt = new

            for name in opt:
                if name != "." and name != "..":
                    flag, block_list = self.workpath.delete_node(name, 2)
                    if not flag:
                        print (Bcolors.FAIL + "Error: {} not exist".format(name))
                    if block_list is not None:
                        self.block.re_blocks(block_list)
                else:
                    print (Bcolors.FAIL + "Error: Can't delete {} .".format(name))
        else:
            opt = opt[1:]
            current_name = self.get_current_name()
            new = list()
            for o in opt:
                if o[0] == '*':
                    o = '.' + o
                for c in current_name:
                    try:
                        optn = re.match(o, c)
                        if optn is not None and optn.group() == optn.string:
                            new.append(optn.group())
                    except:
                        continue
            if len(new) != 0:
                opt = list()
                opt = new
            for name in opt:
                flag, block_list = self.workpath.delete_node(name)
                if not flag:
                    print (Bcolors.FAIL + "Error: {} not exist".format(name))
                if block_list is not None:
                    self.block.re_blocks(block_list)
    
    def tree(self, workpath, lis):
        temp = workpath.diandian
        while temp.next is not None:
            temp = temp.next
            if temp.next is None:
                for i in range(lis[len(lis) - 1]):
                    if i in lis:
                        print ("│", end='')
                    else:
                        print (" ", end='')
                if temp.type == 2:
                    print ("└──" + Bcolors.OKBLUE + temp.name + '/' + Bcolors.ENDC)
                    a = lis.pop()
                    lis.append(a+3)
                    self.tree(temp.dir, lis)
                    a = lis.pop()
                    lis.append(a-3)
                else:
                    print ("└──" + temp.name)
            else:
                for i in range(lis[len(lis) - 1]):
                    if i in lis:
                        print ("│", end='')
                    else:
                        print (" ", end='')
                if temp.type == 2:
                    print ("├──" + Bcolors.OKBLUE + temp.name + '/' + Bcolors.ENDC)
                    lis.append(lis[len(lis) - 1] + 3)
                    self.tree(temp.dir, lis)
                    lis.pop()
                else:
                    print ("├──" + temp.name)

    def show(self, opt):
        if len(opt) < 2:
            self.block.show_super()
        elif opt[1] == '-i':
            if len(opt) < 3:
                print (Bcolors.FAIL + "Error: missing following argument after '-i'")
                print(Bcolors.HEADER + "Useage:\n\tshow [-i] [name]")
            else:
                temp = self.workpath.find_file(opt[2])
                if temp != False:
                    temp.inode.show_block()
                else:
                    print (Bcolors.FAIL + "Error: {} doesn't exist!".format(opt[2]))


    def get_current_name(self, type=1):
        current_name = list()
        temp = self.workpath.head
        while temp.next is not None:
            temp = temp.next
            '''
            type=2 开启返回目录模式，但考虑cd 命令不需要多个匹配结果先不加入。但可以作为TAB自动补全实现用
            if type == 2:
                if temp.type == 2:
                    current_name.append(temp.name)
            '''
            if temp.name != '.' and temp.name != '..':
                current_name.append(temp.name)

        return current_name

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

if __name__ == '__main__':
    shell = Os()
    cmd_list = ['mkdir', 'cd', 'rm', 'touch', 'ls', 'tree', 'cls', 'show']
    cmd_buf = input(Bcolors.DEEPGREEN+ "root@ling: " + Bcolors.OKGREEN + '/' +\
                    shell.get_path() + Bcolors.ENDC + "> ")
    while cmd_buf != "exit":
        cmd = cmd_buf.split()
        c = 0
        flag = 0
        for l in cmd_list:
            if l == cmd[0]:
                flag = 1
                if c == 0:
                    shell.mkdir(cmd)
                elif c == 1:
                    shell.cd(cmd)
                elif c == 2:
                    shell.rm(cmd)
                elif c == 3:
                    shell.touch(cmd)
                elif c == 4:
                    shell.ls(cmd)
                    shell.get_path()
                elif c == 5:
                    lis = [1]
                    shell.tree(shell.workpath, lis)
                elif c == 6:
                    os.system('clear')
                elif c == 7:
                    shell.show(cmd)
            c += 1
        if not flag:
            print (Bcolors.FAIL + "Error: Cmd NoFound" + Bcolors.ENDC)
        cmd_buf = input(Bcolors.DEEPGREEN+ "root@ling: " + Bcolors.OKGREEN + '/' +\
                        shell.get_path() + Bcolors.ENDC + "> ")
