-- Predicate coverage: LIKE, IN, BETWEEN, IS NOT NULL, NOT
SELECT name, department
FROM students
WHERE name LIKE 'A%'
  AND city IN ('Delhi', 'Mumbai', 'Pune')
  AND cgpa BETWEEN 6.0 AND 9.5
  AND department IS NOT NULL
  AND NOT age < 18
ORDER BY 1;
