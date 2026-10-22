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
  var caseCountBadge = document.getElementById("case-count");
  var diagnosticCount = document.getElementById("diagnostic-count");
  var diagnosticsView = document.getElementById("diagnostics-view");
  var astView = document.getElementById("ast-view");
  var astEmpty = document.getElementById("ast-empty");
  var switchInspectorView = document.getElementById("switch-inspector-view");
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

  var EXAMPLES = {
    standard: [
      "switch (status) {",
      "  case 1:",
      "    action = 10;",
      "    break;",
      "  case 2:",
      "    action = 20;",
      "    break;",
      "  case 3:",
      "    action = 30;",
      "    break;",
      "  default:",
      "    action = 0;",
      "    break;",
      "}"
    ].join("\n"),
    duplicate_const: [
      "switch (code) {",
      "  case 1:",
      "    msg = 100;",
      "    break;",
      "  case 2:",
      "    msg = 200;",
      "    break;",
      "  case 1:  // Duplicate constant '1'",
      "    msg = 300;",
      "    break;",
      "  default:",
      "    msg = 0;",
      "    break;",
      "}"
    ].join("\n"),
    duplicate_eval: [
      "switch (value) {",
      "  case 10:",
      "    result = 1;",
      "    break;",
      "  case 2 * 5:  // Evaluates to 10 (duplicate)",
      "    result = 2;",
      "    break;",
      "  case (1 << 4):",
      "    result = 3;",
      "    break;",
      "  default:",
      "    result = 0;",
      "    break;",
      "}"
    ].join("\n"),
    char_ascii: [
      "switch (ch) {",
      "  case 'A':  // ASCII value 65",
      "    type = 1;",
      "    break;",
      "  case 65:   // Duplicate with 'A' (65)",
      "    type = 2;",
      "    break;",
      "  default:",
      "    type = 0;",
      "    break;",
      "}"
    ].join("\n"),
    non_const: [
      "int limit = 5;",
      "switch (x) {",
      "  case limit:  // Non-constant variable identifier",
      "    res = 1;",
      "    break;",
      "  default:",
      "    res = 0;",
      "    break;",
      "}"
    ].join("\n"),
    multi_default: [
      "switch (op) {",
      "  case 1:",
      "    res = 10;",
      "    break;",
      "  default:",
      "    res = 0;",
      "    break;",
      "  default:  // Multiple default labels error",
      "    res = -1;",
      "    break;",
      "}"
    ].join("\n"),
    break_error: [
      "int x = 5;",
      "x = x + 1;",
      "break;  // Error: break statement outside of switch"
    ].join("\n"),
    nested_scope: [
      "switch (outer) {",
      "  case 1:",
      "    switch (inner) {",
      "      case 1:  // Allowed: scoped within inner switch",
      "        val = 10;",
      "        break;",
      "      case 2:",
      "        val = 20;",
      "        break;",
      "    }",
      "    break;",
      "  case 2:",
      "    val = 30;",
      "    break;",
      "  default:",
      "    val = 0;",
      "    break;",
      "}"
    ].join("\n")
  };

  var KEYWORD_TOKENS = [
    "SWITCH", "CASE", "DEFAULT", "BREAK", "INT", "CHAR", "FLOAT", "VOID"
  ];

  var OPERATOR_TOKENS = [
    "PLUS", "MINUS", "STAR", "SLASH", "PERCENT", "ASSIGN", "EQUAL_EQUAL",
    "NOT_EQUAL", "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL",
    "LOGICAL_AND", "LOGICAL_OR", "LOGICAL_NOT", "BIT_AND", "BIT_OR",
    "BIT_XOR", "BIT_NOT", "SHIFT_LEFT", "SHIFT_RIGHT", "LEFT_PAREN",
    "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COLON", "SEMICOLON", "COMMA"
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
    if (type === "INT_LITERAL" || type === "FLOAT_LITERAL" || type === "CHAR_LITERAL" || type === "STRING_LITERAL") {
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
      var lexemeClass = token.type === "STRING_LITERAL" ? "lexeme-string" : "";
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
    input.setSelectionRange(pos, pos);
  }

  function renderDiagnostics(diagnostics) {
    var items = diagnostics || [];
    diagnosticCount.textContent = String(items.length);
    if (items.length === 0) {
      diagnosticsView.innerHTML = "<div class=\"empty-box\"><i data-lucide=\"check-circle-2\" style=\"width: 36px; height: 36px; color: #10b981; margin-bottom: 8px;\"></i><h3>Zero Errors</h3><p>Code passed lexical, syntax, and case-label duplicate validation cleanly.</p></div>";
      refreshIcons();
      return;
    }
    diagnosticsView.innerHTML = items.map(function (item) {
      var info = diagnosticKind(item.code);
      return "<div class=\"diagnostic-card\" onclick=\"window.__jump(" + item.line + ", " + item.column + ")\" style=\"cursor: pointer;\">" +
        "<div class=\"diagnostic-header\">" +
        "<span class=\"code-badge " + info.className + "\">" + escapeHtml(item.code) + "</span>" +
        "<span class=\"location-tag\"><i data-lucide=\"map-pin\" style=\"width: 12px; height: 12px;\"></i> Line " + item.line + ", Col " + item.column + "</span>" +
        "</div>" +
        "<p class=\"diagnostic-message\">" + escapeHtml(item.message) + "</p>" +
        "</div>";
    }).join("");
    refreshIcons();
    if (window.gsap) {
      gsap.fromTo(".diagnostic-card", { opacity: 0, x: -10 }, { opacity: 1, x: 0, duration: 0.25, stagger: 0.05, ease: "power2.out" });
    }
  }

  window.__jump = jumpToLocation;

  function exprToView(expr) {
    if (!expr) return null;
    var meta = "[" + expr.line + ":" + expr.column + "]";
    switch (expr.kind) {
      case "integer":
        return { kind: "literal", label: "IntegerLiteral", meta: meta, detail: String(expr.value) };
      case "float":
        return { kind: "literal", label: "FloatLiteral", meta: meta, detail: String(expr.value) };
      case "char":
        return { kind: "literal", label: "CharLiteral", meta: meta, detail: "'" + expr.char + "' (ASCII " + expr.value + ")" };
      case "string":
        return { kind: "literal", label: "StringLiteral", meta: meta, detail: "\"" + expr.value + "\"" };
      case "identifier":
        return { kind: "column", label: "Identifier", meta: meta, detail: expr.name };
      case "unary":
        return {
          kind: "unary",
          label: "UnaryExpr",
          meta: meta,
          detail: expr.op,
          children: [exprToView(expr.operand)]
        };
      case "binary":
        return {
          kind: "binary",
          label: "BinaryExpr",
          meta: meta,
          detail: expr.op,
          children: [exprToView(expr.left), exprToView(expr.right)]
        };
      case "paren":
        return {
          kind: "clause",
          label: "ParenExpr",
          meta: meta,
          children: [exprToView(expr.inner)]
        };
      default:
        return { kind: "label", label: expr.kind || "Unknown" };
    }
  }

  function stmtToView(stmt) {
    if (!stmt) return null;
    var meta = "[" + stmt.line + ":" + stmt.column + "]";
    switch (stmt.kind) {
      case "switch":
        var children = [];
        if (stmt.condition) {
          children.push({
            kind: "clause",
            label: "Condition",
            children: [exprToView(stmt.condition)]
          });
        }
        if (stmt.body) {
          children.push(stmtToView(stmt.body));
        }
        return {
          kind: "query",
          label: "SwitchStmt",
          meta: meta,
          detail: (stmt.hasDefault ? "default: yes" : "default: none"),
          children: children
        };

      case "case":
        var caseChildren = [];
        if (stmt.expr) {
          caseChildren.push(exprToView(stmt.expr));
        }
        if (stmt.statements && stmt.statements.length > 0) {
          caseChildren.push({
            kind: "clause",
            label: "Statements (" + stmt.statements.length + ")",
            children: stmt.statements.map(stmtToView)
          });
        }
        var detail = "val=" + stmt.constValue;
        if (stmt.isDuplicate) {
          detail += " [DUPLICATE]";
        }
        return {
          kind: stmt.isDuplicate ? "order" : "select",
          label: "CaseLabel",
          meta: meta,
          detail: detail,
          children: caseChildren
        };

      case "default":
        var defChildren = [];
        if (stmt.statements && stmt.statements.length > 0) {
          defChildren.push({
            kind: "clause",
            label: "Statements (" + stmt.statements.length + ")",
            children: stmt.statements.map(stmtToView)
          });
        }
        return {
          kind: "table",
          label: "DefaultLabel",
          meta: meta,
          children: defChildren
        };

      case "break":
        return {
          kind: stmt.isValidContext ? "select" : "order",
          label: "BreakStmt",
          meta: meta,
          detail: stmt.isValidContext ? "valid context" : "MISPLACED"
        };

      case "block":
        return {
          kind: "clause",
          label: "BlockStmt",
          meta: meta,
          detail: (stmt.statements ? stmt.statements.length : 0) + " items",
          children: (stmt.statements || []).map(stmtToView)
        };

      case "assign":
        return {
          kind: "column",
          label: "AssignStmt",
          meta: meta,
          detail: stmt.name + " =",
          children: stmt.value ? [exprToView(stmt.value)] : []
        };

      case "var_decl":
        return {
          kind: "column",
          label: "VarDeclStmt",
          meta: meta,
          detail: stmt.typeName + " " + stmt.name,
          children: stmt.init ? [exprToView(stmt.init)] : []
        };

      case "expr_stmt":
        return {
          kind: "clause",
          label: "ExprStmt",
          meta: meta,
          children: stmt.expr ? [exprToView(stmt.expr)] : []
        };

      case "empty":
        return {
          kind: "label",
          label: "EmptyStmt",
          meta: meta
        };

      default:
        return { kind: "label", label: stmt.kind || "Stmt" };
    }
  }

  function programToView(ast) {
    if (!ast) return null;
    return {
      kind: "query",
      label: "Program",
      meta: "[" + ast.line + ":" + ast.column + "]",
      children: (ast.statements || []).map(stmtToView)
    };
  }

  function renderViewNode(node) {
    var listItem = document.createElement("li");
    var element = document.createElement("div");
    element.className = "tree-node-card kind-" + (node.kind || "clause");

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
    root.appendChild(renderViewNode(programToView(ast)));
    astView.innerHTML = "";
    astView.appendChild(root);
    astView.hidden = false;
    astEmpty.hidden = true;

    if (window.gsap) {
      gsap.fromTo(".tree-node-card", { opacity: 0, scale: 0.96 }, { opacity: 1, scale: 1, duration: 0.25, stagger: 0.02, ease: "power2.out" });
    }
  }

  function renderSwitchInspector(switchAnalysis) {
    var switches = (switchAnalysis && switchAnalysis.switches) || [];
    var totalCases = 0;
    switches.forEach(function (sw) {
      totalCases += (sw.cases || []).length;
    });
    if (caseCountBadge) {
      caseCountBadge.textContent = String(totalCases);
    }

    if (switches.length === 0) {
      switchInspectorView.innerHTML = "<div class=\"empty-box\"><i data-lucide=\"split\" style=\"width: 36px; height: 36px; color: #64748b; margin-bottom: 8px;\"></i><h3>No Switch Statements Detected</h3><p>Write a switch statement in the editor to inspect case constants and duplicate validation.</p></div>";
      refreshIcons();
      return;
    }

    var html = "";
    switches.forEach(function (sw, index) {
      html += "<div class=\"schema-table-card\">" +
        "<div class=\"schema-table-header\">" +
        "<div style=\"display: flex; align-items: center; gap: 8px;\">" +
        "<span class=\"code-badge code-syntax\">Switch #" + (index + 1) + "</span>" +
        "<strong style=\"color: var(--foreground);\">switch (" + escapeHtml(sw.condition || "") + ")</strong>" +
        "</div>" +
        "<span class=\"location-tag\">Line " + sw.line + ", Col " + sw.column + "</span>" +
        "</div>" +
        "<div style=\"padding: 12px 16px; border-bottom: 1px solid var(--border); display: flex; gap: 16px; font-size: 13px; color: var(--muted-foreground);\">" +
        "<span><strong>Cases:</strong> " + (sw.cases ? sw.cases.length : 0) + "</span>" +
        "<span><strong>Default:</strong> " + (sw.hasDefault ? "<span style=\"color: #10b981;\">Present (line " + sw.defaultLine + ")</span>" : "<span style=\"color: #94a3b8;\">None</span>") + "</span>" +
        "</div>" +
        "<table class=\"token-data-table\" style=\"margin: 0;\">" +
        "<thead><tr>" +
        "<th style=\"width: 45px;\">#</th>" +
        "<th>Case Label Expr</th>" +
        "<th>Evaluated Value</th>" +
        "<th>Status</th>" +
        "<th style=\"width: 80px;\">Location</th>" +
        "</tr></thead>" +
        "<tbody>";

      if (!sw.cases || sw.cases.length === 0) {
        html += "<tr><td colspan=\"5\" style=\"text-align: center; color: #64748b; padding: 16px;\">No case labels inside this switch</td></tr>";
      } else {
        sw.cases.forEach(function (c, cIdx) {
          var statusHtml = "";
          if (c.isDuplicate) {
            statusHtml = "<span class=\"token-badge\" style=\"background: rgba(239, 68, 68, 0.15); color: #ef4444; border: 1px solid rgba(239, 68, 68, 0.3);\">DUPLICATE (prev line " + c.prevLine + ")</span>";
          } else if (c.isValidConst) {
            statusHtml = "<span class=\"token-badge\" style=\"background: rgba(16, 185, 129, 0.15); color: #10b981; border: 1px solid rgba(16, 185, 129, 0.3);\">UNIQUE CONSTANT</span>";
          } else {
            statusHtml = "<span class=\"token-badge\" style=\"background: rgba(245, 158, 11, 0.15); color: #f59e0b; border: 1px solid rgba(245, 158, 11, 0.3);\">INVALID CONST</span>";
          }

          html += "<tr onclick=\"window.__jump(" + c.line + ", " + c.column + ")\" style=\"cursor: pointer;\">" +
            "<td style=\"color: #64748b;\">" + (cIdx + 1) + "</td>" +
            "<td><code>case " + escapeHtml(c.exprText) + ":</code></td>" +
            "<td style=\"font-weight: 600; color: #38bdf8;\">" + (c.isValidConst ? c.constValue : "<span style=\"color:#ef4444;\">N/A</span>") + "</td>" +
            "<td>" + statusHtml + "</td>" +
            "<td style=\"color: #94a3b8;\">[" + c.line + ":" + c.column + "]</td>" +
            "</tr>";
        });
      }

      html += "</tbody></table></div>";
    });

    switchInspectorView.innerHTML = html;
    refreshIcons();
  }

  function setPipeline(status) {
    [stepLex, stepParse, stepAst, stepSemantic].forEach(function (el) {
      el.classList.remove("step-success", "step-failed", "step-active");
    });

    if (status === "idle") return;

    if (status === "lexical") {
      stepLex.classList.add("step-failed");
      return;
    }
    stepLex.classList.add("step-success");

    if (status === "syntax") {
      stepParse.classList.add("step-failed");
      return;
    }
    stepParse.classList.add("step-success");
    stepAst.classList.add("step-success");

    if (status === "semantic") {
      stepSemantic.classList.add("step-failed");
      return;
    }
    stepSemantic.classList.add("step-success");
  }

  function updateStatus(badgeClass, badgeText, summaryText) {
    statusBadge.className = "status-badge " + badgeClass;
    statusBadge.textContent = badgeText;
    resultSummary.textContent = summaryText;
  }

  function formatSwitchCode(code) {
    var lines = code.split("\n");
    var indent = 0;
    var formatted = [];
    for (var i = 0; i < lines.length; i++) {
      var trimmed = lines[i].trim();
      if (!trimmed) {
        formatted.push("");
        continue;
      }
      if (trimmed.charAt(0) === "}") {
        indent = Math.max(0, indent - 1);
      }
      var isLabel = trimmed.startsWith("case ") || trimmed.startsWith("default:");
      var currentIndent = isLabel ? Math.max(0, indent - 1) : indent;
      var space = "";
      for (var s = 0; s < currentIndent * 2; s++) space += " ";
      formatted.push(space + trimmed);
      if (trimmed.charAt(trimmed.length - 1) === "{" || trimmed.endsWith("case ") || isLabel && trimmed.endsWith(":")) {
        indent++;
      }
    }
    return formatted.join("\n");
  }

  function analyze() {
    var source = input.value;
    if (!source.trim()) {
      lastResult = null;
      renderTokens([]);
      renderDiagnostics([]);
      renderAst(null);
      renderSwitchInspector(null);
      jsonView.textContent = "// Compiler JSON contract output will appear here";
      updateStatus("status-idle", "IDLE", "Waiting for input");
      setPipeline("idle");
      return;
    }

    if (source.length > MAX_SOURCE_LENGTH) {
      updateStatus("status-error", "OVERSIZED", "Code exceeds 60 KiB limit");
      return;
    }

    var currentSeq = ++requestSequence;
    inFlight = true;
    validateButton.disabled = true;
    updateStatus("status-running", "ANALYZING", "Compiling switch AST & checking case duplicates...");

    fetch("/api/analyze", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ source: source })
    })
      .then(function (res) {
        return res.json().then(function (data) {
          return { ok: res.ok, status: res.status, data: data };
        });
      })
      .then(function (resp) {
        if (currentSeq !== requestSequence) return;
        inFlight = false;
        validateButton.disabled = false;

        if (!resp.ok) {
          var err = (resp.data && resp.data.error) || { code: "B001", message: "Server error" };
          lastResult = null;
          renderDiagnostics([{ code: err.code, line: 1, column: 1, message: err.message }]);
          updateStatus("status-error", "BRIDGE ERROR", err.message);
          setPipeline("idle");
          showTab("diagnostics");
          return;
        }

        var result = resp.data;
        lastResult = result;

        jsonView.textContent = JSON.stringify(result, null, 2);
        renderTokens(result.tokens);
        renderDiagnostics(result.diagnostics);
        renderAst(result.ast);
        renderSwitchInspector(result.switchAnalysis);

        if (result.ok) {
          updateStatus("status-success", "VALID", "Switch statements & case labels are strictly valid");
          setPipeline("valid");
        } else if (result.stage === "lexical") {
          updateStatus("status-error", "LEXICAL ERROR", "Illegal character or unterminated literal");
          setPipeline("lexical");
          showTab("diagnostics");
        } else if (result.stage === "syntax") {
          updateStatus("status-error", "SYNTAX ERROR", "Grammar mismatch in switch construct");
          setPipeline("syntax");
          showTab("diagnostics");
        } else if (result.stage === "semantic") {
          var dupCount = (result.diagnostics || []).filter(function (d) { return d.code === "S004"; }).length;
          var summary = dupCount > 0 ? "Detected " + dupCount + " duplicate case label(s)" : "Semantic validation failed";
          updateStatus("status-error", "SEMANTIC ERROR", summary);
          setPipeline("semantic");
          showTab("switch-inspector");
        }
      })
      .catch(function (err) {
        if (currentSeq !== requestSequence) return;
        inFlight = false;
        validateButton.disabled = false;
        updateStatus("status-error", "OFFLINE", "Cannot reach local compiler bridge");
        bridgeStatus.className = "badge badge-offline";
        bridgeStatus.innerHTML = "<span class=\"badge-pulse-dot\"></span><span>Offline</span>";
      });
  }

  function checkHealth() {
    fetch("/api/health")
      .then(function (res) { return res.json(); })
      .then(function (data) {
        if (data.ok && data.compilerAvailable) {
          bridgeStatus.className = "badge badge-online";
          bridgeStatus.innerHTML = "<span class=\"badge-pulse-dot\"></span><span>Bridge Active</span>";
        } else {
          bridgeStatus.className = "badge badge-offline";
          bridgeStatus.innerHTML = "<span class=\"badge-pulse-dot\"></span><span>Compiler Missing</span>";
        }
      })
      .catch(function () {
        bridgeStatus.className = "badge badge-offline";
        bridgeStatus.innerHTML = "<span class=\"badge-pulse-dot\"></span><span>Bridge Offline</span>";
      });
  }

  // Event Listeners
  input.addEventListener("input", function () {
    updateEditorMeter();
  });

  input.addEventListener("keydown", function (e) {
    if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
      e.preventDefault();
      analyze();
    }
  });

  validateButton.addEventListener("click", function () {
    analyze();
  });

  clearButton.addEventListener("click", function () {
    input.value = "";
    updateEditorMeter();
    analyze();
  });

  formatButton.addEventListener("click", function () {
    if (input.value.trim()) {
      input.value = formatSwitchCode(input.value);
      updateEditorMeter();
    }
  });

  copySqlButton.addEventListener("click", function () {
    navigator.clipboard.writeText(input.value).then(function () {
      copySqlText.textContent = "Copied!";
      setTimeout(function () { copySqlText.textContent = "Copy"; }, 1500);
    });
  });

  copyAstButton.addEventListener("click", function () {
    if (lastResult) {
      navigator.clipboard.writeText(JSON.stringify(lastResult, null, 2)).then(function () {
        copyAstText.textContent = "Copied!";
        setTimeout(function () { copyAstText.textContent = "Copy"; }, 1500);
      });
    }
  });

  document.querySelectorAll(".example-chip").forEach(function (chip) {
    chip.addEventListener("click", function () {
      var key = chip.getAttribute("data-example");
      if (EXAMPLES[key]) {
        input.value = EXAMPLES[key];
        updateEditorMeter();
        analyze();
      }
    });
  });

  document.querySelectorAll(".tab").forEach(function (tab) {
    tab.addEventListener("click", function () {
      showTab(tab.getAttribute("data-tab"));
    });
  });

  // Initialization
  input.value = EXAMPLES.duplicate_eval;
  updateEditorMeter();
  refreshIcons();
  checkHealth();
  analyze();

})();
