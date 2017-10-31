all: netfilter_block

netfilter_block: nfqnl_test.c
	gcc -o netfilter_block nfqnl_test.c -lnetfilter_queue
