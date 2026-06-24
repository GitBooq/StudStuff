CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    email VARCHAR (255) UNIQUE NOT NULL,
    date TIMESTAMPTZ DEFAULT NOW(),

    CONSTRAINT email_not_empty CHECK (length(trim(email)) > 0)
);


CREATE TABLE IF NOT EXISTS posts (
    id SERIAL PRIMARY KEY,
    title VARCHAR (255) NOT NULL, 
    description VARCHAR (255), -- Allow null decription
    date TIMESTAMPTZ DEFAULT NOW(),
    status VARCHAR (20) NOT NULL,
    author_id INT, -- can be NULL

    CONSTRAINT fk_users_posts FOREIGN KEY (author_id) REFERENCES users (id) ON DELETE SET NULL,
    CONSTRAINT check_status CHECK (status IN ('draft', 'published', 'archived')),
    CONSTRAINT title_not_empty CHECK (length(trim(title)) > 0)
);

CREATE TABLE IF NOT EXISTS comments (
    id SERIAL PRIMARY KEY,
    content VARCHAR (255) NOT NULL,   
    date TIMESTAMPTZ DEFAULT NOW(),     
    author_id INT,   -- can be NULL
    post_id INT NOT NULL,

    CONSTRAINT fk_users_comments FOREIGN KEY (author_id) REFERENCES users (id) ON DELETE SET NULL,
    CONSTRAINT fk_posts_comments FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE,
    CONSTRAINT content_not_empty CHECK (length(trim(content)) > 0)
);

CREATE TABLE IF NOT EXISTS tags (
    id SERIAL PRIMARY KEY,
    name VARCHAR (255) NOT NULL UNIQUE,

    CONSTRAINT name_not_empty CHECK (length(trim(name)) > 0)
);

-- junction table
-- n-to-n between posts and tags
CREATE TABLE IF NOT EXISTS post_tags (
    id SERIAL PRIMARY KEY,
    post_id INT NOT NULL,
    tag_id INT NOT NULL,

    CONSTRAINT fk_posts_post_tags FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE,
    CONSTRAINT fk_tags_post_tags FOREIGN KEY (tag_id) REFERENCES tags (id) ON DELETE CASCADE,
    CONSTRAINT unique_post_tag UNIQUE (post_id, tag_id)
);

