# Building on python script from https://github.com/Vxer-Lee/AndroidPin-Crack
# Adding a function to support passphrases 

import os
import struct
import scrypt
import hashlib
import binascii
import itertools

#------------------------Global Variable-----------------------
N = 16384
r = 8
p = 1
#--------------------------------------------------------------


def Test(signature,meta,salt,password):
	password = binascii.unhexlify(password);
	to_hash = meta
	to_hash += password
	hash = scrypt.hash(to_hash, salt, N, r, p)

	print 'signature  %s' % signature.encode('hex')
	print 'Hash:      %s' % hash[0:32].encode('hex')
	print 'Equal:     %s' % (hash[0:32] == signature)
	pass


def PatternBrute(signature,meta,salt):
	to_hash = meta
	matrix=[]
	for i in range(0,9):
	    str_temp = '0'+str(i)
	    matrix.append(str_temp)

	#The characters of 00-08 are arranged, at least 4 Numbers are arranged, and all are arranged at most.
	min_num=4
	max_num=len(matrix)

	for num in range(min_num,max_num+1):#04 -> 08
	    iter1 = itertools.permutations(matrix,num)#Pick "n" of the "9" Numbers.
	    list_m=[]
	    list_m.append(list(iter1))#Store the resulting array in the list_m list.
	    for el in list_m[0]:#I'm going to go through all of these n Numbers.
	        strlist=''.join(el)#list to str. [00,03,06,07,08]-->0003060708
	        to_hash = meta
	        to_hash += strlist
	        hash = scrypt.hash(to_hash, salt, N, r, p)
	        if(hash[0:32] == signature):
			print 'Password',strlist
	pass

# New function
def PasswordBrute(signature,meta,salt):
	# Change to rock you txt
	test = ["password","12345","god","abcd","marijane","spongebob","blah","letmein","summer69","lifeinchinatown","foobar"]
	password = ""
	to_hash = meta
	hash = scrypt.hash(to_hash, salt, N, r, p)
	for i in range(0,len(hash)):
		password = test[i]
		to_hash = meta
		to_hash += password
		hash = scrypt.hash(to_hash, salt, N, r, p)
		#sucess!!!!!!!!!!!!!!
		if(hash[0:32] == signature):
			print "password:"+ test[i]
			break
		print test[i]
	pass

def Pin4NumBrute(signature,meta,salt):
	stri = ""
	password = stri
	to_hash = meta
	hash = scrypt.hash(to_hash, salt, N, r, p)
	for i in range(0,10000):
		stri=str(i)
		if(i<1000):
			if(i%1000<=9):
				stri="0"+"0"+"0"+str(i)
			elif(i%1000<=99):
				stri="0"+"0"+str(i)
			else:
				stri="0"+str(i)
		password = stri
		to_hash = meta
		to_hash += password
		hash = scrypt.hash(to_hash, salt, N, r, p)
		#sucess!!!!!!!!!!!!!!
		if(hash[0:32] == signature):
			print "password:"+stri
			break
		print stri
	pass



def Pin6NumBrute(signature,meta,salt):
	stri = ""
	password = stri
	to_hash = meta
	hash = scrypt.hash(to_hash, salt, N, r, p)
	for i in range(0,1000000):
		stri=str(i)
		if(i<100000):
			if(i%100000<=9):
				stri="0"+"0"+"0"+"0"+"0"+str(i)
			elif(i%100000<=99):
				stri="0"+"0"+"0"+"0"+str(i)
			elif(i%100000<=999):
				stri="0"+"0"+"0"+str(i)
			elif(i%100000<=9999):
				stri="0"+"0"+str(i)
			else:
				stri="0"+str(i)
		password = stri
		to_hash = meta
		to_hash += password
		hash = scrypt.hash(to_hash, salt, N, r, p)
		#sucess!!!!!!!!!!!!!!
		if(hash[0:32] == signature):
			print "password:"+stri
			break
		print stri
	pass


def main():
	print '-----'
	f = open('gatekeeper.password.key', 'rb')
	blob = f.read()
	s = struct.Struct('<'+'17s 8s 32s')
	(meta, salt, signature) = s.unpack_from(blob)
	print '===============key Hash===================='
	print '%s' % (signature.encode('hex'))
	print '==========================================='
	print ''
	print ''
	print '------------>Brute force now!!!!!----------'
	PasswordBrute(signature,meta,salt)
	#Test(signature,meta,salt,"00010204060708")

if __name__ == "__main__":
    main()
