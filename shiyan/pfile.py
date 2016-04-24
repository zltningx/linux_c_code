#!/usr/bin/env python
# coding=utf-8

import re
import os
from datetime import datetime

class Linkedfile:
    class _Node:
        __slots__ = 'name', 'size', 'first_block', 'type', 'datetime', 'prior', 'next', 'dir'
        def __init__(self, name, size, first_block, type, prior, next, dir):
            self.name = name
            self.size = size
            self.first_block = first_block
            self.type = type
            self.datetime = datetime.now().strftime('%c')
            self.prior = prior
            self.next = next
            self.dir = dir

    def __init__(self):
        self.head = self._Node("root", 0, 0, -1, None, None, None)

        self.dian = self._Node(".", 0, 0, 2, self.head, None, self)
        self.diandian = self._Node("..", 0, 0, 2, self.dian, None, self)
        self.dian.next = self.diandian

        self.head.next = self.dian
        self.prior = None

    def append(self, name, size, first_block, type=1, dir=None):
        #查重
        temp = self.head
        while temp.next is not None:
            temp = temp.next
            if temp.name == name:
                print (Bcolors.FAIL + "Error: file or dirent exist")
                return


        #尾插法
        temp = self.head
        new = self._Node(name, size, first_block, type, None, None, dir)

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
                        return True
                pre = temp.prior
                succ = temp.next
                pre.next = succ
                if succ is not None:
                    succ.prior = pre
                return True
        return False

    def show_list(self):
        temp = self.head
        while temp.next != None:
            temp = temp.next
            if not temp.name.startswith("."):
                if temp.type == 1:
                    print ("- {:<2d} {:<5d} {} {}".format(temp.first_block, temp.size, \
                                                          temp.datetime, temp.name))
                if temp.type == 2:
                    print ("d {:<2d} {:<5d} {} ".format(temp.first_block, temp.size, \
                                                        temp.datetime), end='')
                    print (Bcolors.OKBLUE + "{}/".format(temp.name) + Bcolors.ENDC)

    def show_all(self):
        temp = self.head
        while temp.next != None:
            temp = temp.next
            if temp.type == 1:
                print ("- {:<2d} {:<5d} {} {}".format(temp.first_block, temp.size,\
                                                        temp.datetime, temp.name))
            if temp.type == 2:
                    print ("d {:<2d} {:<5d} {} ".format(temp.first_block, temp.size,\
                                                          temp.datetime), end='')
                    print (Bcolors.OKBLUE + "{}/".format(temp.name) + Bcolors.ENDC)

class Os:
    def __init__(self):
        self.root = Linkedfile()
        self.workpath = self.root

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
        if opt[1] == '/':
            self.workpath = self.root
            return
        flag = 0
        temp = self.workpath.head
        while temp.next is not None:
            temp = temp.next
            if temp.name == opt[1]:
                flag = 1
                if temp.type != 2:
                    return Bcolors.FAIL + "Error: {} is not dirent".format(opt[1])
                self.workpath = temp.dir
        if not flag:
            print (Bcolors.FAIL + "Error: {} not exist".format(opt[1]))

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
            self.workpath.append(opt[1], int(opt[2]), 0)
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
                self.workpath.append(name, 1, 0, 2, dir)
        else:
            print (Bcolors.FAIL + "Error: not enough option")
            print (Bcolors.HEADER + "Useage:\n\tmkdir [dir_name] ...")

    def rm(self,opt):
        if len(opt) < 2:
            print(Bcolors.FAIL + "Error: not enough option")
            print(Bcolors.HEADER + "Useage:\n\trm [name] ...\n\trm -r [dir_name] ...")
        elif opt[1] == '-r':
            opt = opt[2:]
            for name in opt:
                if name != "." and name != "..":
                    flag = self.workpath.delete_node(name, 2)
                    if not flag:
                        print (Bcolors.FAIL + "Error: {} not exist".format(name))
                else:
                    print (Bcolors.FAIL + "Error: Can't delete {} .".format(name))
        else:
            opt = opt[1:]
            for name in opt:
                flag = self.workpath.delete_node(name)
                if not flag:
                    print (Bcolors.FAIL + "Error: {} not exist".format(name))
            
    def tree(self, workpath, blank):
        temp = workpath.head
        while temp.next is not None:
            temp = temp.next
            if temp.name.startswith('.'):
                continue
            if temp.type == 2:
                print ("  " * blank + "|___" + Bcolors.OKBLUE + temp.name + '/' + Bcolors.ENDC)
                self.tree(temp.dir, blank+2)
            else:
                print ("  "* blank + "|___" + temp.name)

    def cls(self):
        os.system('clear')

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
    cmd_list = ['mkdir', 'cd', 'rm', 'touch', 'ls', 'tree', 'cls']
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
                    shell.tree(shell.root, 2)
                elif c == 6:
                    shell.cls()
            c += 1
        if not flag:
            print (Bcolors.FAIL + "Error: Cmd NoFound" + Bcolors.ENDC)
        cmd_buf = input(Bcolors.DEEPGREEN+ "root@ling: " + Bcolors.OKGREEN + '/' +\
                        shell.get_path() + Bcolors.ENDC + "> ")
