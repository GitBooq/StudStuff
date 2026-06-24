-- Для каждого пользователя получить набор тэгов от постов, к которым пользователь оставлял комментарий.
SELECT
    U.id AS user_id,
    U.email,
    ARRAY_AGG(DISTINCT T.name ORDER BY T.name) AS tags
FROM users U
LEFT JOIN comments C ON C.author_id=U.id
LEFT JOIN post_tags PT ON PT.post_id=C.post_id
LEFT JOIN tags T ON T.id=PT.tag_id
GROUP BY U.id, U.email
ORDER BY U.id
LIMIT 50;

-- Выполнить безопасную вставку нового пользователя и поста от его имени (с помощью транзакции)
DO $$
DECLARE
    new_user_id INT;
    new_user_email TEXT := 'd34db33f@example.com';
BEGIN
    INSERT INTO users (email) VALUES (new_user_email)
    ON CONFLICT (email) DO NOTHING
    RETURNING id INTO new_user_id;

    IF new_user_id IS NOT NULL THEN
        INSERT INTO posts (title, description, status, author_id)
        VALUES ('transaction test', 'test', 'published', new_user_id);

        RAISE NOTICE 'New user and post have been created.';
    ELSE
        RAISE NOTICE 'A user with email ''%'' already exists.', new_user_email;
    END IF;
EXCEPTION
    WHEN OTHERS THEN
        RAISE NOTICE 'Error: %', SQLERRM;
        ROLLBACK;
END $$;

-- Проанализировать узлы выполнения запроса на поиск по совпадению LIKE %content% в посте.
-- Написать короткий вывод и рекомендацию по добавлению индексов.

-- Enable trigrams
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX IF NOT EXISTS idx_comments_content_trgm ON comments USING GIN (content gin_trgm_ops);

ANALYZE comments;

-- Temporal index disable
SET enable_indexscan = off;
SET enable_bitmapscan = off;

EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM comments WHERE content LIKE '%g3m%';

-- Revert index scan
SET enable_indexscan = on;
SET enable_bitmapscan = on;

EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM comments WHERE content LIKE '%g3m%';