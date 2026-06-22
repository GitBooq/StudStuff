.PHONY: up down logs import generate build-cpp-client run-cpp-client quiz clean

up:
	docker compose up -d postgres

down:
	docker compose down

logs:
	docker compose logs -f

generate:
	docker compose --profile generate up --build data-gen

import:
	docker compose exec -T postgres psql -U root -d mydb < scripts/import.sql

quiz:
	docker compose exec -T postgres psql -U root -d mydb < scripts/quiz.sql

build-cpp-client:
	docker compose build cpp-client

run-cpp-client:
	docker compose run --rm cpp-client

clean:
	docker compose down -v
	rm -rf data/*.csv

all: up generate import
