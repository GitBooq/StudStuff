# Working with PostgreSQL: Schema Design, Constraints, and C++ Client Development
## Requirements
- Docker (w/ *compose*)
- Make
## Quick Start
```bash
make all            # build cpp client; generate data; create and setup dbms
make quiz           # run SQL query script
make run-cpp-client # return node; // return node
```

*Data generation settings can be adjusted using env vars*

## SQL query script

**See scripts in/output of quiz.sql**

``` bash
"Для каждого пользователя получить набор тэгов от постов, к которым пользователь оставлял комментарий."
```
---
``` bash
"Выполнить безопасную вставку нового пользователя и поста от его имени (с помощью транзакции)"
```
---
``` bash
"Проанализировать узлы выполнения запроса на поиск по совпадению LIKE %content% в посте. Написать короткий вывод и рекомендацию по добавлению индексов."
```

**Sequential scan (w/o indexing)**
``` bash
                                                QUERY PLAN
----------------------------------------------------------------------------------------------------------
 Seq Scan on comments  (cost=0.00..4394.00 rows=10 width=221) (actual time=18.387..18.388 rows=1 loops=1)
   Filter: ((content)::text ~~ '%g3m%'::text)
   Rows Removed by Filter: 99999
   Buffers: shared hit=3144
 Planning:
   Buffers: shared hit=42
 Planning Time: 0.288 ms
 Execution Time: 18.397 ms
(8 rows)

```
---
**Bitmap Heap + Index Scan**
``` bash
                                                             QUERY PLAN
------------------------------------------------------------------------------------------------------------------------------------
 Bitmap Heap Scan on comments  (cost=21.11..59.54 rows=10 width=221) (actual time=0.009..0.010 rows=1 loops=1)
   Recheck Cond: ((content)::text ~~ '%g3m%'::text)
   Heap Blocks: exact=1
   Buffers: shared hit=4
   ->  Bitmap Index Scan on idx_comments_content_trgm  (cost=0.00..21.11 rows=10 width=0) (actual time=0.005..0.005 rows=1 loops=1)
         Index Cond: ((content)::text ~~ '%g3m%'::text)
         Buffers: shared hit=3
 Planning:
   Buffers: shared hit=1
 Planning Time: 0.143 ms
 Execution Time: 0.028 ms
(11 rows)
```
**Conclusion:** LIKE '%text%' search requires a trigram-based index (pg_trgm) to speed up custom search.
