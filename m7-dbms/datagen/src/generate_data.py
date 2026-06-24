from __future__ import annotations

import csv
import os
import random
from dataclasses import dataclass
from pathlib import Path

from dotenv import load_dotenv
from faker import Faker


FAKER = Faker("en_US")

DATA_DIR = Path("data")

STATUSES = [
    "draft",
    "published",
    "archived",
]

MY_COMMENT = "This is the g3m comment."


@dataclass(frozen=True)
class Config:
    num_users: int
    num_posts: int
    num_comments: int
    num_tags: int
    num_post_tags: int


def load_config() -> Config:
    """Load configuration from environment variables."""

    load_dotenv()

    return Config(
        num_users=int(os.getenv("NUM_USERS", 100)),
        num_posts=int(os.getenv("NUM_POSTS", 50)),
        num_comments=int(os.getenv("NUM_COMMENTS", 1000)),
        num_tags=int(os.getenv("NUM_TAGS", 10)),
        num_post_tags=int(os.getenv("NUM_POST_TAGS", 150)),
    )


def random_datetime(
    min_months: int,
    max_months: int,
):
    """Generate random datetime in specified month range."""

    return FAKER.date_time_between(
        start_date=f"-{random.randint(min_months, max_months)}m",
        end_date="now",
    )


def generate_users(
    filename: Path,
    count: int,
) -> None:
    """Generate users.csv."""

    print(f"Generating {count} users...")

    used_emails: set[str] = set()

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(
            file,
            delimiter=",",
            quoting=csv.QUOTE_MINIMAL,
        )

        user_id = 1

        while user_id <= count:
            email = FAKER.email()

            if email in used_emails:
                continue

            used_emails.add(email)

            created_at = random_datetime(12, 24)

            writer.writerow(
                [
                    user_id,
                    email,
                    created_at.strftime("%Y-%m-%d %H:%M:%S"),
                ]
            )

            user_id += 1

    print(
        f"✓ Users saved to {filename} "
        f"({count} unique emails)"
    )


def generate_posts(
    filename: Path,
    count: int,
    num_users: int,
) -> None:
    """Generate posts.csv."""

    print(f"Generating {count} posts...")

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(
            file,
            delimiter=",",
            quoting=csv.QUOTE_MINIMAL,
        )

        for post_id in range(1, count + 1):
            title = (
                FAKER.sentence(
                    nb_words=random.randint(3, 8)
                )
                .rstrip(".")
            )

            description = None

            if random.random() < 0.7:
                description = FAKER.paragraph(
                    nb_sentences=random.randint(1, 3)
                )

            created_at = random_datetime(6, 18)

            status = random.choices(
                STATUSES,
                weights=[0.2, 0.6, 0.2],
            )[0]

            author_id = (
                random.randint(1, num_users)
                if random.random() < 0.7
                else None
            )

            writer.writerow(
                [
                    post_id,
                    title,
                    description,
                    created_at.strftime("%Y-%m-%d %H:%M:%S"),
                    status,
                    author_id,
                ]
            )

    print(f"✓ Posts saved to {filename}")


def generate_comments(
    filename: Path,
    count: int,
    num_users: int,
    num_posts: int,
) -> None:
    """Generate comments.csv."""

    print(f"Generating {count} comments...")

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(
            file,
            delimiter=",",
            quoting=csv.QUOTE_MINIMAL,
        )

        for comment_id in range(1, count + 1):
            content = FAKER.text(max_nb_chars=250)

            created_at = random_datetime(1, 12)

            author_id = (
                random.randint(1, num_users)
                if random.random() < 0.8
                else None
            )

            post_id = random.randint(1, num_posts)

            writer.writerow(
                [
                    comment_id,
                    content,
                    created_at.strftime("%Y-%m-%d %H:%M:%S"),
                    author_id,
                    post_id,
                ]
            )

    print(f"✓ Comments saved to {filename}")


def add_my_comment_to_csv(
    filename: Path,
    num_users: int,
    num_posts: int,
) -> None:
    """Append predefined comment."""

    with filename.open(
        "r",
        encoding="utf-8",
    ) as file:
        comments = list(csv.reader(file))

    new_comment_id = len(comments) + 1

    created_at = random_datetime(1, 12)

    comments.append(
        [
            str(new_comment_id),
            MY_COMMENT,
            created_at.strftime("%Y-%m-%d %H:%M:%S"),
            str(random.randint(1, num_users)),
            str(random.randint(1, num_posts)),
        ]
    )

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(
            file,
            delimiter=",",
            quoting=csv.QUOTE_MINIMAL,
        )
        writer.writerows(comments)

    print(f"✓ Added my comment to {filename}")


def generate_tags(
    filename: Path,
    count: int,
) -> None:
    """Generate tags.csv."""

    print(f"Generating {count} tags...")

    tag_categories = {
        "Technology": [
            "AI",
            "Machine Learning",
            "Cloud Computing",
            "DevOps",
            "Cybersecurity",
            "Data Science",
            "Blockchain",
            "IoT",
            "5G",
            "Quantum Computing",
        ],
        "Science": [
            "Physics",
            "Biology",
            "Chemistry",
            "Astronomy",
            "Geology",
            "Genetics",
            "Mathematics",
            "Neuroscience",
            "Ecology",
        ],
        "Business": [
            "Startups",
            "Marketing",
            "Leadership",
            "Strategy",
            "Finance",
            "Innovation",
            "Entrepreneurship",
            "Management",
            "Analytics",
        ],
        "Lifestyle": [
            "Health",
            "Travel",
            "Food",
            "Fitness",
            "Relationships",
            "Productivity",
            "Mindfulness",
            "Wellness",
            "Nutrition",
        ],
        "Culture": [
            "Art",
            "Literature",
            "History",
            "Music",
            "Cinema",
            "Philosophy",
            "Poetry",
            "Theater",
            "Photography",
        ],
        "Programming": [
            "Python",
            "JavaScript",
            "Rust",
            "Go",
            "React",
            "Node.js",
            "SQL",
            "Docker",
            "Kubernetes",
            "AWS",
            "Git",
            "Linux",
        ],
    }

    all_tags: list[str] = []

    for category_tags in tag_categories.values():
        all_tags.extend(category_tags)

    all_tags = list(set(all_tags))

    used_tags = set(all_tags)
    generated_tags: list[str] = []

    while len(all_tags) + len(generated_tags) < count:
        tag = FAKER.word().capitalize()

        if tag not in used_tags and len(tag) > 2:
            used_tags.add(tag)
            generated_tags.append(tag)

    all_tags.extend(generated_tags)

    random.shuffle(all_tags)

    selected_tags = all_tags[:count]

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(file)

        for tag_id, tag in enumerate(
            selected_tags,
            start=1,
        ):
            writer.writerow([tag_id, tag])

    print(
        f"✓ Tags saved to {filename} "
        f"({len(selected_tags)} unique tags)"
    )


def generate_post_tags(
    filename: Path,
    count: int,
    num_posts: int,
    num_tags: int,
) -> None:
    """Generate post_tags.csv."""

    print(
        f"Generating {count} post-tag relationships..."
    )

    unique_pairs: set[tuple[int, int]] = set()

    with filename.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as file:
        writer = csv.writer(file)

        for post_id in range(1, num_posts + 1):
            selected_tags = random.sample(
                range(1, num_tags + 1),
                random.randint(1, 5),
            )

            for tag_id in selected_tags:
                pair = (post_id, tag_id)

                if pair in unique_pairs:
                    continue

                unique_pairs.add(pair)

                writer.writerow(
                    [
                        len(unique_pairs),
                        post_id,
                        tag_id,
                    ]
                )

                if len(unique_pairs) >= count:
                    print(
                        f"✓ Post-tags saved to "
                        f"{filename} "
                        f"(reached {count} records)"
                    )
                    return

    print(
        f"✓ Post-tags saved to {filename} "
        f"({len(unique_pairs)} records)"
    )


def generate_all_data() -> None:
    """Generate all CSV datasets."""

    DATA_DIR.mkdir(
        parents=True,
        exist_ok=True,
    )

    config = load_config()

    print("=" * 60)
    print("STARTING DATA GENERATION")
    print("=" * 60)

    generate_users(
        DATA_DIR / "users.csv",
        config.num_users,
    )

    generate_posts(
        DATA_DIR / "posts.csv",
        config.num_posts,
        config.num_users,
    )

    generate_tags(
        DATA_DIR / "tags.csv",
        config.num_tags,
    )

    generate_comments(
        DATA_DIR / "comments.csv",
        config.num_comments,
        config.num_users,
        config.num_posts,
    )

    add_my_comment_to_csv(
        DATA_DIR / "comments.csv",
        config.num_users,
        config.num_posts,
    )

    generate_post_tags(
        DATA_DIR / "post_tags.csv",
        config.num_post_tags,
        config.num_posts,
        config.num_tags,
    )

    print("\nGenerated files:")
    print(f"  - users.csv: {config.num_users} users")
    print(f"  - posts.csv: {config.num_posts} posts")
    print(f"  - comments.csv: {config.num_comments} comments")
    print(f"  - tags.csv: {config.num_tags} tags")
    print(
        "  - post_tags.csv: "
        "relationships between posts and tags"
    )


if __name__ == "__main__":
    generate_all_data()