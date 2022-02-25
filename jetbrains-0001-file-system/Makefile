# Run 'make' for complete build from scratch + running the tests.
all: clean lib test run

clean:
	rm -rf build
	rm -rf workspace

lib:
	javac -g -d build src/lightfs/*

test:
	javac -g -classpath build -d build src/tests/*

run:
	rm -rf workspace
	mkdir workspace
	cd workspace && java -classpath ../build -ea tests/Tests

rerun: test run

debug:
	rm -rf workspace
	mkdir workspace
	cd workspace && jdb -ea -classpath ../build -sourcepath ../src tests/Tests
