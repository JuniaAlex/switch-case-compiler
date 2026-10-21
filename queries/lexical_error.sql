-- Expected: L002 (unterminated string)
SELECT name
FROM students
WHERE name = 'Bob;
