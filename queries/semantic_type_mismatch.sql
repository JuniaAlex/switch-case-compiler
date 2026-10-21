-- Semantic Error S005: Type mismatch (comparing INTEGER age with TEXT 'twenty')
SELECT name
FROM students
WHERE age = 'twenty';
