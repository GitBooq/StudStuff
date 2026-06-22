-- ============================================
-- ИМПОРТ ДАННЫХ В POSTGRESQL
-- ============================================

BEGIN;

-- Отключаем проверки для ускорения импорта
SET session_replication_role = replica;

-- 1. Импорт пользователей
\COPY users(id, email, date) FROM 'data/users.csv' DELIMITER ',' CSV HEADER;

-- 2. Импорт тегов
\COPY tags(id, name) FROM 'data/tags.csv' DELIMITER ',' CSV HEADER;

-- 3. Импорт постов
\COPY posts(id, title, description, date, status, author_id) FROM 'data/posts.csv' DELIMITER ',' CSV HEADER;

-- 4. Импорт комментариев
\COPY comments(id, content, date, author_id, post_id) FROM 'data/comments.csv' DELIMITER ',' CSV HEADER;

-- 5. Импорт связей постов и тегов
\COPY post_tags(id, post_id, tag_id) FROM 'data/post_tags.csv' DELIMITER ',' CSV HEADER;

-- Включаем проверки обратно
SET session_replication_role = DEFAULT;

-- ============================================
-- ОБНОВЛЕНИЕ ПОСЛЕДОВАТЕЛЬНОСТЕЙ (SERIAL)
-- ============================================

-- Обновляем последовательности для автоинкремента
SELECT setval('users_id_seq', (SELECT MAX(id) FROM users));
SELECT setval('tags_id_seq', (SELECT MAX(id) FROM tags));
SELECT setval('posts_id_seq', (SELECT MAX(id) FROM posts));
SELECT setval('comments_id_seq', (SELECT MAX(id) FROM comments));
SELECT setval('post_tags_id_seq', (SELECT MAX(id) FROM post_tags));

-- ============================================
-- ПРОВЕРКА ДАННЫХ (опционально)
-- ============================================

-- Проверяем количество записей
SELECT 'users' as table_name, COUNT(*) as count FROM users
UNION ALL
SELECT 'posts', COUNT(*) FROM posts
UNION ALL
SELECT 'comments', COUNT(*) FROM comments
UNION ALL
SELECT 'tags', COUNT(*) FROM tags
UNION ALL
SELECT 'post_tags', COUNT(*) FROM post_tags;

-- Показываем примеры данных
SELECT 'Users sample:' as info;
SELECT * FROM users LIMIT 5;

SELECT 'Posts sample:' as info;
SELECT * FROM posts LIMIT 5;

SELECT 'Comments sample:' as info;
SELECT * FROM comments LIMIT 5;

SELECT 'Tags sample:' as info;
SELECT * FROM tags LIMIT 5;

SELECT 'Post-Tags sample:' as info;
SELECT * FROM post_tags LIMIT 5;

-- Проверяем связи
SELECT 
    'Total posts with authors:' as info,
    COUNT(*) as count 
FROM posts 
WHERE author_id IS NOT NULL;

SELECT 
    'Total comments with authors:' as info,
    COUNT(*) as count 
FROM comments 
WHERE author_id IS NOT NULL;

SELECT 
    'Posts with tags:' as info,
    COUNT(DISTINCT post_id) as count 
FROM post_tags;

SELECT 
    'Average tags per post:' as info,
    ROUND(AVG(tag_count), 2) as avg_tags
FROM (
    SELECT post_id, COUNT(*) as tag_count 
    FROM post_tags 
    GROUP BY post_id
) as tag_counts;

COMMIT;