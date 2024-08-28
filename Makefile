
.PHONY: compile deploy

compile:
	emcc riskdice.c -O3 -o site/riskdice.js \
		-sEXPORTED_FUNCTIONS=_malloc,_free \
		-sEXPORTED_RUNTIME_METHODS=ccall,setValue,getValue \
		-sMODULARIZE

deploy:
	rm -f website.tar.gz
	tar -czf website.tar.gz site
	scp website.tar.gz masflam@masflam.com:/var/www/riskdice.masflam.com/website.tar.gz
	ssh masflam@masflam.com 'cd /var/www/riskdice.masflam.com; [ ! -d site ] && mkdir site; rm -rf site_prev && mv site site_prev && tar -xzf website.tar.gz && rm website.tar.gz'
