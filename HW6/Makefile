all: out/client out/server

run_client: out/client
	$<

run_server: out/server
	$<

out/server.o: src/server.c out
	gcc -c $< -O3 -o $@

out/multiplex.o: src/multiplex.c out
	gcc -c $< -O3 -o $@

out/multiplex.a: out/multiplex.o
	ar rcs $@ $<

out/client: src/client.c out/multiplex.a out
	gcc $< out/multiplex.a -O3 -o $@

out/server: out/server.o out/multiplex.a out
	gcc $< out/multiplex.a -O3 -o $@

out:
	mkdir $@

clean:
	rm -rf out
