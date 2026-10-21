(function () {
  "use strict";

  // DOM Elements
  var input = document.getElementById("sql-input");
  var validateButton = document.getElementById("validate-button");
  var clearButton = document.getElementById("clear-button");
  var formatButton = document.getElementById("format-button");
  var copySqlButton = document.getElementById("copy-sql-button");
  var copySqlText = document.getElementById("copy-sql-text");
  var copyAstButton = document.getElementById("copy-ast-button");
  var copyAstText = document.getElementById("copy-ast-text");
  var lineNumbers = document.getElementById("line-numbers");
  var editorLength = document.getElementById("editor-length");
  var editorHint = document.getElementById("editor-hint");
  var statusBadge = document.getElementById("status-badge");
  var resultSummary = document.getElementById("result-summary");
  var tokenBody = document.getElementById("token-body");
  var tokenCount = document.getElementById("token-count");
  var diagnosticCount = document.getElementById("diagnostic-count");
  var diagnosticsView = document.getElementById("diagnostics-view");
  var astView = document.getElementById("ast-view");
  var astEmpty = document.getElementById("ast-empty");
  var schemaView = document.getElementById("schema-view");
  var jsonView = document.getElementById("json-view");
  var bridgeStatus = document.getElementById("bridge-status");

  // Pipeline Steps
  var stepLex = document.getElementById("step-lex");
  var stepParse = document.getElementById("step-parse");
  var stepAst = document.getElementById("step-ast");
  var stepSemantic = document.getElementById("step-semantic");

  var inFlight = false;
  var requestSequence = 0;
  var lastResult = null;
  var MAX_SOURCE_LENGTH = 60000;

  var CATALOG = [
    {
      name: "students",
      columns: [
        { name: "student_id", type: "INTEGER", pk: true },
        { name: "name", type: "TEXT" },
        { name: "age", type: "INTEGER" },
        { name: "department", type: "TEXT" },
        { name: "cgpa", type: "FLOAT" },
        { name: "city", type: "TEXT" }
      ]
    },
    {
      name: "courses",
      columns: [
        { name: "course_id", type: "INTEGER", pk: true },
        { name: "title", type: "TEXT" },
        { name: "credits", type: "INTEGER" },
        { name: "department", type: "TEXT" }
      ]
    },
    {
      name: "enrollments",
      columns: [
        { name: "enrollment_id", type: "INTEGER", pk: true },
        { name: "student_id", type: "INTEGER" },
        { name: "course_id", type: "INTEGER" },
        { name: "grade", type: "TEXT" },
        { name: "semester", type: "INTEGER" }
      ]
    },
    {
      name: "departments",
      columns: [
        { name: "department", type: "TEXT", pk: true },
        { name: "hod", type: "TEXT" },
        { name: "building", type: "TEXT" },
        { name: "budget", type: "FLOAT" }
      ]
    }
  ];

  var EXAMPLES = {
    join: [
      "SELECT s.name AS student_name, c.title, e.grade",
      "FROM students s, enrollments e, courses c",
      "WHERE s.student_id = e.student_id",
      "  AND e.course_id = c.course_id",
      "  AND s.cgpa >= 7.5",
      "ORDER BY s.cgpa DESC, s.name;"
    ].join("\n"),
    filters: [
      "SELECT DISTINCT name, cgpa * 2 AS double_cgpa",
      "FROM students",
      "WHERE age BETWEEN 18 AND 25",
      "  AND city IN ('Delhi', 'Mumbai', 'Pune')",
      "  AND name NOT LIKE 'X%'",
      "  AND department IS NOT NULL",
      "ORDER BY cgpa DESC;"
    ].join("\n"),
    ambiguous: [
      "SELECT department, name",
      "FROM students, courses",
      "WHERE students.department = courses.department",
      "ORDER BY department;"
    ].join("\n"),
    syntax: [
      "SELECT name, age",
      "FROM students",
      "WHERE (age > 18",
      "ORDER BY age;"
    ].join("\n"),
    lexical: [
      "SELECT name, cgpa",
      "FROM students",
      "WHERE name = 'Alice;"
    ].join("\n")
  };

  var KEYWORD_TOKENS = [
    "SELECT", "DISTINCT", "FROM", "WHERE", "ORDER", "BY", "ASC", "DESC",
    "AND", "OR", "NOT", "AS", "IN", "BETWEEN", "LIKE", "IS", "NULL", "TRUE", "FALSE"
  ];

  var OPERATOR_TOKENS = [
    "STAR", "PLUS", "MINUS", "SLASH", "EQUAL", "NOT_EQUAL", "LESS", "LESS_EQUAL",
    "GREATER", "GREATER_EQUAL", "COMMA", "DOT", "LEFT_PAREN", "RIGHT_PAREN", "SEMICOLON"
  ];

  function refreshIcons() {
    if (window.lucide && typeof window.lucide.createIcons === "function") {
      try {
        window.lucide.createIcons();
      } catch (e) {
        // Safe fallback
      }
    }
  }

  function escapeHtml(value) {
    return String(value)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function visibleTokens(tokens) {
    return (tokens || []).filter(function (token) {
      return token.type !== "EOF";
    });
  }

  function countPhase(diagnostics, prefix) {
    return (diagnostics || []).filter(function (item) {
      return String(item.code).charAt(0) === prefix;
    }).length;
  }

  function updateLineNumbers() {
    if (!lineNumbers || !input) return;
    var lines = input.value.split("\n");
    var lineCount = Math.max(lines.length, 1);
    var numbersHtml = "";
    for (var i = 1; i <= lineCount; i++) {
      numbersHtml += i + "<br>";
    }
    lineNumbers.innerHTML = numbersHtml;
  }

  function updateEditorMeter() {
    var length = input.value.length;
    editorLength.textContent = length + " character" + (length === 1 ? "" : "s");
    updateLineNumbers();
  }

  function showTab(name) {
    document.querySelectorAll(".tab").forEach(function (tab) {
      var active = tab.getAttribute("data-tab") === name;
      tab.classList.toggle("active", active);
      tab.setAttribute("aria-selected", active ? "true" : "false");
    });
    document.querySelectorAll(".tab-panel").forEach(function (panel) {
      var active = panel.id === "panel-" + name;
      panel.classList.toggle("active", active);
      if (active && window.gsap) {
        gsap.fromTo(panel, { opacity: 0, y: 6 }, { opacity: 1, y: 0, duration: 0.25, ease: "power2.out" });
      }
    });
    refreshIcons();
  }

  function tokenClass(type) {
    if (KEYWORD_TOKENS.indexOf(type) !== -1) {
      return "token-keyword";
    }
    if (type === "INT_LITERAL" || type === "FLOAT_LITERAL" || type === "STRING") {
      return "token-literal";
    }
    if (OPERATOR_TOKENS.indexOf(type) !== -1) {
      return "token-operator";
    }
    return "";
  }

  function renderTokens(tokens) {
    var visible = visibleTokens(tokens);
    tokenCount.textContent = String(visible.length);
    if (visible.length === 0) {
      tokenBody.innerHTML = "<tr><td colspan=\"5\" style=\"text-align: center; padding: 30px; color: #64748b;\">No tokens analyzed yet</td></tr>";
      return;
    }
    tokenBody.innerHTML = visible.map(function (token, index) {
      var lexemeClass = token.type === "STRING" ? "lexeme-string" : "";
      var cls = tokenClass(token.type);
      var badge = cls ? "<span class=\"token-badge " + cls + "\">" + escapeHtml(token.type) + "</span>" : escapeHtml(token.type);
      return "<tr>" +
        "<td style=\"color: #64748b;\">" + (index + 1) + "</td>" +
        "<td>" + badge + "</td>" +
        "<td class=\"" + lexemeClass + "\">" + escapeHtml(token.lexeme) + "</td>" +
        "<td style=\"color: #94a3b8;\">" + token.line + "</td>" +
        "<td style=\"color: #94a3b8;\">" + token.column + "</td>" +
        "</tr>";
    }).join("");

    if (window.gsap) {
      gsap.fromTo("#token-body tr", { opacity: 0, y: 4 }, { opacity: 1, y: 0, duration: 0.2, stagger: 0.015, ease: "power1.out" });
    }
  }

  function diagnosticKind(code) {
    var prefix = String(code).charAt(0);
    if (prefix === "L") {
      return { className: "code-lex", title: "Lexical Error", icon: "file-x" };
    }
    if (prefix === "P") {
      return { className: "code-syntax", title: "Syntax Error", icon: "bug" };
    }
    if (prefix === "B") {
      return { className: "code-syntax", title: "Bridge Error", icon: "alert-triangle" };
    }
    return { className: "code-semantic", title: "Semantic Error", icon: "shield-alert" };
  }

  function jumpToLocation(line, column) {
    if (!input || line <= 0) return;
    var lines = input.value.split("\n");
    var pos = 0;
    for (var i = 0; i < line - 1 && i < lines.length; i++) {
      pos += lines[i].length + 1;
    }
    pos += Math.max(0, column - 1);
    input.focus();
    input.setSelectionRange(pos, pos + 1);
  }

  function renderDiagnostics(diagnostics) {
    diagnosticCount.textContent = String(diagnostics.length);
    diagnosticCount.classList.toggle("has-errors", diagnostics.length > 0);

    if (diagnostics.length === 0) {
      diagnosticsView.innerHTML =
        "<div class=\"success-box\">" +
        "<div class=\"success-icon-circle\"><i data-lucide=\"check-circle-2\" style=\"width: 28px; height: 28px;\"></i></div>" +
        "<h3>All Validations Passed</h3>" +
        "<p>No lexical, syntax, or semantic diagnostics were detected by the C11 compiler.</p>" +
        "</div>";
      refreshIcons();
      return;
    }

    diagnosticsView.innerHTML = diagnostics.map(function (item) {
      var kind = diagnosticKind(item.code);
      return "<article class=\"diagnostic-alert " + kind.className + "\" onclick=\"window.__jumpToLocation(" + item.line + ", " + item.column + ")\" style=\"cursor: pointer;\" title=\"Click to focus error location in editor\">" +
        "<span class=\"diag-code-badge\">" + escapeHtml(item.code) + "</span>" +
        "<div class=\"diag-content\">" +
        "<h4>" + kind.title + "</h4>" +
        "<p>" + escapeHtml(item.message) + "</p>" +
        "<div class=\"diag-pos-pill\"><i data-lucide=\"map-pin\" style=\"width: 12px; height: 12px;\"></i> Line " + item.line + ", Column " + item.column + "</div>" +
        "</div>" +
        "</article>";
    }).join("");

    refreshIcons();

    if (window.gsap) {
      gsap.fromTo(".diagnostic-alert", { opacity: 0, x: -10 }, { opacity: 1, x: 0, duration: 0.3, stagger: 0.08, ease: "power2.out" });
    }
  }

  window.__jumpToLocation = jumpToLocation;

  function literalDetail(expression) {
    if (expression.literalKind === "string") {
      return "string '" + expression.value + "'";
    }
    if (expression.literalKind === "null") {
      return "null";
    }
    if (expression.literalKind === "boolean") {
      return "boolean " + (expression.value ? "TRUE" : "FALSE");
    }
    return expression.literalKind + " " + expression.value;
  }

  function expressionView(expression) {
    if (!expression) {
      return null;
    }
    var position = "[" + expression.line + ":" + expression.column + "]";
    if (expression.kind === "literal") {
      return { kind: "literal", label: "Literal", meta: position, detail: literalDetail(expression) };
    }
    if (expression.kind === "column") {
      var columnName = expression.qualifier ? expression.qualifier + "." + expression.name : expression.name;
      return { kind: "column", label: "Column", meta: position, detail: columnName };
    }
    if (expression.kind === "unary") {
      return {
        kind: "unary",
        label: "Unary",
        meta: position,
        detail: expression.op,
        children: [expressionView(expression.operand)]
      };
    }
    if (expression.kind === "binary") {
      return {
        kind: "binary",
        label: "Binary",
        meta: position,
        detail: expression.op,
        children: [expressionView(expression.left), expressionView(expression.right)]
      };
    }
    if (expression.kind === "like") {
      return {
        kind: "binary",
        label: "Like",
        meta: position,
        detail: expression.negated ? "NOT LIKE" : "LIKE",
        children: [
          { kind: "label", label: "Value", children: [expressionView(expression.value)] },
          { kind: "label", label: "Pattern", children: [expressionView(expression.pattern)] }
        ]
      };
    }
    if (expression.kind === "in") {
      return {
        kind: "binary",
        label: "In",
        meta: position,
        detail: expression.negated ? "NOT IN" : "IN",
        children: [
          { kind: "label", label: "Value", children: [expressionView(expression.value)] },
          {
            kind: "label",
            label: "Items (" + expression.items.length + ")",
            children: expression.items.map(expressionView)
          }
        ]
      };
    }
    if (expression.kind === "between") {
      return {
        kind: "binary",
        label: "Between",
        meta: position,
        detail: expression.negated ? "NOT BETWEEN" : "BETWEEN",
        children: [
          { kind: "label", label: "Value", children: [expressionView(expression.value)] },
          { kind: "label", label: "Lower", children: [expressionView(expression.lower)] },
          { kind: "label", label: "Upper", children: [expressionView(expression.upper)] }
        ]
      };
    }
    if (expression.kind === "is_null") {
      return {
        kind: "binary",
        label: "IsNull",
        meta: position,
        detail: expression.negated ? "IS NOT NULL" : "IS NULL",
        children: [expressionView(expression.value)]
      };
    }
    return { kind: "label", label: expression.kind || "Unknown" };
  }

  function astToView(ast) {
    var children = [];

    children.push({
      kind: "clause",
      label: "SelectList",
      meta: ast.selectList.length + " item" + (ast.selectList.length === 1 ? "" : "s"),
      children: ast.selectList.map(function (item) {
        if (item.star) {
          return {
            kind: "select",
            label: "SelectItem",
            meta: "[" + item.line + ":" + item.column + "]",
            detail: "*"
          };
        }
        return {
          kind: "select",
          label: "SelectItem",
          meta: "[" + item.line + ":" + item.column + "]",
          detail: item.alias ? "AS " + item.alias : "",
          children: [expressionView(item.expression)]
        };
      })
    });

    children.push({
      kind: "clause",
      label: "FromList",
      meta: ast.fromList.length + " table" + (ast.fromList.length === 1 ? "" : "s"),
      children: ast.fromList.map(function (table) {
        return {
          kind: "table",
          label: "TableRef",
          meta: "[" + table.line + ":" + table.column + "]",
          detail: table.name + (table.alias ? " AS " + table.alias : "")
        };
      })
    });

    if (ast.where) {
      children.push({
        kind: "clause",
        label: "Where",
        children: [expressionView(ast.where)]
      });
    }

    if (ast.orderList.length > 0) {
      children.push({
        kind: "clause",
        label: "OrderList",
        meta: ast.orderList.length + " item" + (ast.orderList.length === 1 ? "" : "s"),
        children: ast.orderList.map(function (item) {
          return {
            kind: "order",
            label: "OrderItem",
            meta: "[" + item.line + ":" + item.column + "]",
            detail: item.ascending ? "ASC" : "DESC",
            children: [expressionView(item.expression)]
          };
        })
      });
    }

    return {
      kind: "query",
      label: "Query",
      meta: "[" + ast.line + ":" + ast.column + "]",
      detail: ast.distinct ? "DISTINCT" : "",
      children: children
    };
  }

  function renderViewNode(node) {
    var listItem = document.createElement("li");
    var element = document.createElement("div");
    element.className = "tree-node-card kind-" + node.kind;

    var html = "<span class=\"tree-node-kind\">" + escapeHtml(node.label) + "</span>";
    if (node.meta) {
      html += "<span class=\"tree-node-meta\">" + escapeHtml(node.meta) + "</span>";
    }
    if (node.detail) {
      html += "<span class=\"tree-node-detail\">" + escapeHtml(node.detail) + "</span>";
    }
    element.innerHTML = html;
    listItem.appendChild(element);

    if (node.children && node.children.length > 0) {
      var childList = document.createElement("ul");
      node.children.forEach(function (child) {
        if (child) {
          childList.appendChild(renderViewNode(child));
        }
      });
      listItem.appendChild(childList);
    }
    return listItem;
  }

  function renderAst(ast) {
    if (!ast) {
      astEmpty.hidden = false;
      astView.hidden = true;
      astView.innerHTML = "";
      return;
    }
    var root = document.createElement("ul");
    root.appendChild(renderViewNode(astToView(ast)));
    astView.innerHTML = "";
    astView.appendChild(root);
    astView.hidden = false;
    astEmpty.hidden = true;

    if (window.gsap) {
      gsap.fromTo(".tree-node-card", { opacity: 0, scale: 0.95 }, { opacity: 1, scale: 1, duration: 0.25, stagger: 0.02, ease: "back.out(1.2)" });
    }
  }

  function insertIntoEditor(text) {
    if (!input) return;
    var start = input.selectionStart;
    var end = input.selectionEnd;
    input.setRangeText(text, start, end, "end");
    input.focus();
    updateEditorMeter();
  }

  window.__insertIntoEditor = insertIntoEditor;

  function renderSchema() {
    schemaView.innerHTML = CATALOG.map(function (table) {
      var columns = table.columns.map(function (column) {
        var typeClass = column.type === "FLOAT" ? "type-float" :
          column.type === "TEXT" ? "type-text" : "type-int";
        var pkBadge = column.pk ? "<i data-lucide=\"key\" style=\"width: 12px; height: 12px; color: #fbbf24; margin-right: 4px;\"></i>" : "";
        return "<div class=\"schema-col-row\" onclick=\"window.__insertIntoEditor('" + escapeHtml(column.name) + "')\" title=\"Click to insert '" + escapeHtml(column.name) + "' into query\">" +
          "<span>" + pkBadge + escapeHtml(column.name) + "</span>" +
          "<span class=\"type-pill " + typeClass + "\">" + column.type + "</span>" +
          "</div>";
      }).join("");

      return "<article class=\"schema-table-card\">" +
        "<div class=\"schema-table-head\">" +
        "<span class=\"schema-table-name\" onclick=\"window.__insertIntoEditor('" + escapeHtml(table.name) + "')\" style=\"cursor: pointer;\" title=\"Click to insert table name\"><i data-lucide=\"table\" style=\"width: 16px; height: 16px; color: #818cf8;\"></i>" + escapeHtml(table.name) + "</span>" +
        "<span class=\"badge badge-outline\" style=\"font-size: 0.68rem;\">" + table.columns.length + " cols</span>" +
        "</div>" +
        "<div class=\"schema-cols-list\">" + columns + "</div>" +
        "</article>";
    }).join("");

    refreshIcons();
  }

  function updatePipelineSteps(stage, ok) {
    var steps = [stepLex, stepParse, stepAst, stepSemantic];
    steps.forEach(function (s) {
      if (s) s.classList.remove("active-step");
    });

    if (stage === "analyzing") {
      if (stepLex) stepLex.classList.add("active-step");
      return;
    }

    if (ok) {
      steps.forEach(function (s) {
        if (s) s.classList.add("active-step");
      });
      return;
    }

    if (stage === "lexical") {
      if (stepLex) stepLex.classList.add("active-step");
    } else if (stage === "syntax") {
      if (stepLex) stepLex.classList.add("active-step");
      if (stepParse) stepParse.classList.add("active-step");
    } else if (stage === "semantic") {
      if (stepLex) stepLex.classList.add("active-step");
      if (stepParse) stepParse.classList.add("active-step");
      if (stepAst) stepAst.classList.add("active-step");
      if (stepSemantic) stepSemantic.classList.add("active-step");
    }
  }

  function renderSummary(result) {
    var statusText;
    var summary;
    var className;
    var tokens = visibleTokens(result.tokens).length;
    var diagnostics = result.diagnostics || [];
    var tableCount = result.ast && result.ast.fromList ? result.ast.fromList.length : 0;

    if (result.ok) {
      statusText = "VALID";
      className = "status-valid";
      summary = tokens + " tokens \u00b7 " + tableCount + " table" + (tableCount === 1 ? "" : "s") + " \u00b7 0 diagnostics";
    } else if (result.stage === "lexical") {
      statusText = "LEXICAL ERROR";
      className = "status-warn";
      summary = countPhase(diagnostics, "L") + " lexical diagnostic(s) \u00b7 " + tokens + " tokens";
    } else if (result.stage === "syntax") {
      statusText = "SYNTAX ERROR";
      className = "status-error";
      summary = countPhase(diagnostics, "P") + " syntax diagnostic(s) \u00b7 " + tokens + " tokens";
    } else {
      statusText = "SEMANTIC ERROR";
      className = "status-error";
      summary = countPhase(diagnostics, "S") + " semantic diagnostic(s) \u00b7 " + tokens + " tokens";
    }

    statusBadge.textContent = statusText;
    statusBadge.className = "status-badge " + className;
    resultSummary.textContent = summary;
    updatePipelineSteps(result.stage, result.ok);
  }

  function renderResult(result) {
    lastResult = result;
    renderSummary(result);
    renderTokens(result.tokens);
    renderAst(result.ast);
    renderDiagnostics(result.diagnostics || []);
    if (jsonView) {
      jsonView.textContent = JSON.stringify(result, null, 2);
    }
    if (result.ok) {
      showTab("ast");
    } else {
      showTab("diagnostics");
    }
  }

  function showBridgeError(message) {
    statusBadge.textContent = "BRIDGE ERROR";
    statusBadge.className = "status-badge status-error";
    resultSummary.textContent = message;
    renderTokens([]);
    renderAst(null);
    renderDiagnostics([{
      code: "B001",
      line: 0,
      column: 0,
      message: message
    }]);
    if (jsonView) {
      jsonView.textContent = JSON.stringify({ error: { code: "B001", message: message } }, null, 2);
    }
    showTab("diagnostics");
    editorHint.textContent = "Could not reach the local compiler bridge";
    updatePipelineSteps(null, false);
  }

  function setAnalyzing() {
    statusBadge.textContent = "ANALYZING";
    statusBadge.className = "status-badge status-idle";
    resultSummary.textContent = "Running Flex scanner, Bison parser and semantic validator...";
    editorHint.textContent = "Compiling through local C11 bridge";
    updatePipelineSteps("analyzing", false);
  }

  async function analyze(source) {
    var controller = new AbortController();
    var timeout = setTimeout(function () {
      controller.abort();
    }, 10000);
    try {
      var response = await fetch("/api/analyze", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ source: source }),
        signal: controller.signal
      });
      var payload;
      try {
        payload = await response.json();
      } catch (error) {
        throw new Error("The bridge returned an invalid response.");
      }
      if (!response.ok) {
        if (payload && payload.error && payload.error.message) {
          throw new Error(payload.error.message);
        }
        throw new Error("The bridge rejected the request (HTTP " + response.status + ").");
      }
      return payload;
    } finally {
      clearTimeout(timeout);
    }
  }

  async function validate() {
    if (inFlight) {
      return;
    }
    if (input.value.length > MAX_SOURCE_LENGTH) {
      showBridgeError("Query exceeds the 60,000 character bridge limit.");
      return;
    }

    if (window.gsap && validateButton) {
      gsap.to(validateButton, { scale: 0.95, duration: 0.1, yoyo: true, repeat: 1 });
    }

    var sequence = ++requestSequence;
    inFlight = true;
    validateButton.disabled = true;
    setAnalyzing();
    updateEditorMeter();

    try {
      var result = await analyze(input.value);
      if (sequence !== requestSequence) {
        return;
      }
      renderResult(result);
      editorHint.textContent = result.ok ? "Query accepted by the Flex/Bison compiler" : "See diagnostics for details";
    } catch (error) {
      if (sequence !== requestSequence) {
        return;
      }
      var message = error && error.name === "AbortError"
        ? "The compiler request timed out."
        : (error && error.message ? error.message : "Could not reach the local compiler bridge.");
      showBridgeError(message + " Start the bridge with: python backend/server.py");
    } finally {
      if (sequence === requestSequence) {
        inFlight = false;
        validateButton.disabled = false;
      }
    }
  }

  function setExample(name) {
    input.value = EXAMPLES[name];
    updateEditorMeter();
    validate();
    input.focus();
  }

  function formatSql() {
    var text = input.value.trim();
    if (!text) return;
    // Clean, modern SQL formatting
    var formatted = text
      .replace(/\s+/g, " ")
      .replace(/\bSELECT\b/gi, "SELECT")
      .replace(/\bFROM\b/gi, "\nFROM")
      .replace(/\bWHERE\b/gi, "\nWHERE")
      .replace(/\bORDER\s+BY\b/gi, "\nORDER BY")
      .replace(/\bAND\b/gi, "\n  AND")
      .replace(/\bOR\b/gi, "\n  OR")
      .replace(/\bDISTINCT\b/gi, "DISTINCT")
      .replace(/,\s*/g, ", ")
      .replace(/;\s*$/g, ";");
    input.value = formatted;
    updateEditorMeter();
    if (window.gsap) {
      gsap.fromTo(input, { opacity: 0.6 }, { opacity: 1, duration: 0.2 });
    }
  }

  function copyToClipboard(text, button, textSpan) {
    if (!navigator.clipboard) return;
    navigator.clipboard.writeText(text).then(function () {
      if (textSpan) {
        var original = textSpan.textContent;
        textSpan.textContent = "Copied!";
        setTimeout(function () {
          textSpan.textContent = original;
        }, 1500);
      }
    });
  }

  function reset() {
    requestSequence++;
    inFlight = false;
    validateButton.disabled = false;
    input.value = "";
    updateEditorMeter();
    tokenCount.textContent = "0";
    diagnosticCount.textContent = "0";
    diagnosticCount.classList.remove("has-errors");
    tokenBody.innerHTML = "<tr><td colspan=\"5\" style=\"text-align: center; padding: 30px; color: #64748b;\">No tokens analyzed yet</td></tr>";
    diagnosticsView.innerHTML = "";
    astView.innerHTML = "";
    astView.hidden = true;
    astEmpty.hidden = false;
    if (jsonView) jsonView.textContent = "// Compiler JSON contract output will appear here";
    statusBadge.textContent = "IDLE";
    statusBadge.className = "status-badge status-idle";
    resultSummary.textContent = "Waiting for input";
    editorHint.textContent = "Ctrl + Enter to validate";
    updatePipelineSteps(null, false);
    showTab("ast");
    input.focus();
  }

  function checkBridge() {
    fetch("/api/health").then(function (response) {
      return response.json();
    }).then(function (health) {
      if (health.compilerAvailable) {
        bridgeStatus.innerHTML = "<span class=\"badge-pulse-dot\"></span><span>Bridge online</span>";
        bridgeStatus.className = "badge badge-online";
      } else {
        bridgeStatus.innerHTML = "<span>Compiler missing</span>";
        bridgeStatus.className = "badge badge-offline";
      }
    }).catch(function () {
      bridgeStatus.innerHTML = "<span>Bridge offline</span>";
      bridgeStatus.className = "badge badge-offline";
    });
  }

  // Event Listeners
  validateButton.addEventListener("click", validate);
  clearButton.addEventListener("click", reset);

  if (formatButton) {
    formatButton.addEventListener("click", formatSql);
  }

  if (copySqlButton) {
    copySqlButton.addEventListener("click", function () {
      copyToClipboard(input.value, copySqlButton, copySqlText);
    });
  }

  if (copyAstButton) {
    copyAstButton.addEventListener("click", function () {
      var content = lastResult ? JSON.stringify(lastResult, null, 2) : (astView.innerText || "");
      copyToClipboard(content, copyAstButton, copyAstText);
    });
  }

  document.querySelectorAll(".tab").forEach(function (tab) {
    tab.addEventListener("click", function () {
      showTab(tab.getAttribute("data-tab"));
    });
  });

  document.querySelectorAll(".example-chip").forEach(function (chip) {
    chip.addEventListener("click", function () {
      setExample(chip.getAttribute("data-example"));
    });
  });

  input.addEventListener("input", updateEditorMeter);

  // Sync scrolling between textarea and line gutter
  input.addEventListener("scroll", function () {
    if (lineNumbers) {
      lineNumbers.scrollTop = input.scrollTop;
    }
  });

  input.addEventListener("keydown", function (event) {
    if ((event.ctrlKey || event.metaKey) && event.key === "Enter") {
      event.preventDefault();
      validate();
      return;
    }
    if (event.key === "Tab") {
      event.preventDefault();
      var start = input.selectionStart;
      var end = input.selectionEnd;
      input.setRangeText("  ", start, end, "end");
      updateEditorMeter();
    }
  });

  // Entrance animations via GSAP
  if (window.gsap) {
    gsap.from(".app-header", { opacity: 0, y: -20, duration: 0.5, ease: "power2.out" });
    gsap.from(".pipeline-bar", { opacity: 0, y: -10, duration: 0.5, delay: 0.1, ease: "power2.out" });
    gsap.from(".card-panel", { opacity: 0, y: 20, duration: 0.6, delay: 0.2, stagger: 0.15, ease: "power3.out" });
  }

  renderSchema();
  checkBridge();
  input.value = EXAMPLES.join;
  updateEditorMeter();
  validate();
})();
