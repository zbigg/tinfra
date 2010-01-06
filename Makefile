# TODO: rewrite as script, makefile is not needed here
LOCAL_DB=tinfra.mtn
TAILOR=tailor -v --source-repository $(shell mtn au get_option database)

all: run_tailor

run_tailor:
	$(TAILOR) -c tinfra.tailor
	$(TAILOR) -c tinfra-support.tailor
	$(TAILOR) -c tinfra-ssh.tailor
	$(TAILOR) -c tinfra-regexp.tailor

tinfra-rootdir/svnside-pl.reddix.tinfra:
	svn co https://tinfra.googlecode.com/svn/trunk/tinfra $@

tinfra-rootdir/mtnside-pl.reddix.tinfra:
	mtn -b pl.reddix.tinfra co $@

.PHONY: run_tailor

