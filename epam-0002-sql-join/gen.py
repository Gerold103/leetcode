import argparse
from random import randint

# Benchmark files generator. Usage example:
#
#     gen.py -t1 10000 -t2 1000 -t3 1000 -max 1000
#
# Then run the main program in the same folder as these files and watch the
# output.

parser = argparse.ArgumentParser(description='Generate test files')
parser.add_argument('-t1', type=int, help='Size of t1 table', required=False, default=5)
parser.add_argument('-t2', type=int, help='Size of t2 table', required=False, default=5)
parser.add_argument('-t3', type=int, help='Size of t3 table', required=False, default=5)
parser.add_argument('-max', type=int, help='Max value', required=False, default=100)
args = parser.parse_args()

def write_table(name, count):
	t1 = open(name, 'w')
	t1.write('{}\n'.format(count))
	for i in range(count):
		if i != 0:
			t1.write('\n')
		t1.write('{} {}'.format(randint(0, args.max), randint(0, args.max)))
	t1.close()

write_table('t1', args.t1)
write_table('t2', args.t2)
write_table('t3', args.t3)
