# TODO: rewrite as script, makefile is not needed here
LOCAL_DB=tinfra.mtn
TAILOR=tailor -v --source-repository $(shell mtn au get_option database)

all: run_tailor

run_tailor:
	$(TAILOR) -c tinfra.tailor
	$(TAILOR) -c tinfra-support.tailor
	$(TAILOR) -c tinfra-ssh.tailor
	$(TAILOR) -c tinfra-regexp.tailor

.PHONY: run_tailor

