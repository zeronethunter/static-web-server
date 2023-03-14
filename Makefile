.PHONY: build
build: ## Build server
	mkdir build
	cd build
	cmake ..
	make

.PHONY: run
run: build ## Run server
	cd build && ./static-web-server

.PHONY: clear
clear: ## Clear build directory
	rm -rf build

.PHONY: docker_build_server
docker_build_server: clear ## Docker build server
	docker build -f Dockerfile -t server:latest .

.PHONY: docker_start_server
docker_start_server: clear docker_build_server ## Run server container
	docker run --rm -p 80:80 -v $(pwd)/benchmark/server/config.conf:/etc/httpd.conf -v $(pwd)/httptest:/var/www/html/httptest --name server -t server

.PHONY: docker_start_nginx
docker_start_nginx: ## Run nginx container
	docker run --rm -p 8080:8080 -v $(pwd)/benchmark/nginx/nginx.conf:/etc/nginx/nginx.conf -v $(pwd)/httptest:/var/www/html/httptest --name nginx-test -t nginx

.PHONY: docker_run_it_server
docker_run_it_server: clear docker_build_server ## Docker run server in interactive mode
	docker run -it --rm server /bin/bash

.PHONY: perf_test_server_ab
perf_test_server_ab: ## Run ab performance test for server
	ab -n 20000 -c 250 127.0.0.1:80/httptest/wikipedia_russia.html > benchmark/server/ab_benchmark.txt

.PHONY: perf_test_server_wrk
perf_test_server_wrk: ## Run wrk performance test for server
	wrk --latency -d30s http://127.0.0.1:80/httptest/wikipedia_russia.html >benchmark/server/wrk_benchmark.txt

.PHONY: perf_test_nginx_ab
perf_test_nginx_ab: ## Run ab performance test for nginx
	ab -n 20000 -c 250 127.0.0.1:8080/httptest/wikipedia_russia.html >benchmark/nginx/ab_nginx.txt

.PHONY: perf_test_nginx_wrk
perf_test_nginx_wrk: ## Run wrk performance test for nginx
	wrk --latency -d30s http://127.0.0.1:8080/httptest/wikipedia_russia.html >benchmark/nginx/wrk_nginx.txt

.PHONY: benchmark_server
benchmark_server: perf_test_server_ab perf_test_server_wrk ## Reload benchmark for server

.PHONY: benchmark_nginx
benchmark_nginx: perf_test_nginx_ab perf_test_nginx_wrk ## Reload benchmark for nginx

.PHONY: help
help: ## Get commands help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

.DEFAULT_GOAL := help
