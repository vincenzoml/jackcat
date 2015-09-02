

jackcat: jackcat.c
	gcc $< -o $@ -l jack

clean:
	rm -f *~ \#* jackcat
