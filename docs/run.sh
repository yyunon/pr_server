# To run the server
./src/pr_server/pr_server 0.0.0.0 1234 16

# An example command
ab -n 100000 -c 1000 -g bigFile1GProto.tsv 0.0.0.0:1234/bigFileProto > bigFile1GProto.txt
