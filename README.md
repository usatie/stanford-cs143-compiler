# StanfordOnline SOE.YCSCS1 : Compilers
## Set up (on Ubuntu 24.04.2 LTS instance on AWS)
```
# Linux setup instruction
sudo apt-get update
sudo apt-get install flex bison build-essential csh libxaw7-dev
sudo mkdir /usr/class
sudo chown $USER /usr/class
cd /usr/class
mkdir cs143
cd cs143
wget https://courses.edx.org/asset-v1:StanfordOnline+SOE.YCSCS1+1T2020+type@asset+block@student-dist.tar.gz
mv asset-v1\:StanfordOnline+SOE.YCSCS1+1T2020+type@asset+block@student-dist.tar.gz student-dist.tar.gz
tar -xf student-dist.tar.gz
rm student-dist.tar.gz
git init
git add . 
git commit -m "Initial Commit"
git remote add origin git@github.com:usatie/stanford-cs143-compiler.git
git branch -M main
git push -u origin main

# Export PATH
echo "export PATH=/usr/class/cs143/bin:$PATH" >> ~/.bash_profile

# Fix spim error
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install gcc-multilib

```

Checked everything works correctly
```
$ cd examples
$ coolc list.cl
$ spim -file list.s
SPIM Version 6.5 of January 4, 2003
Copyright 1990-2003 by James R. Larus (larus@cs.wisc.edu).
All Rights Reserved.
See the file README for a full copyright notice.
Loaded: ../lib/trap.handler
5 4 3 2 1
4 3 2 1
3 2 1
2 1
1
COOL program successfully executed
```
