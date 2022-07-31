/* transcompiled from https://github.com/antonmedv/codejar */
/* Daniel Starke: modified for better support with PrismJS */
"use strict";
function withLineNumbers(highlight, options = {}) {
    const opts = Object.assign({ class: "codejar-linenumbers", wrapClass: "codejar-wrap", width: "35px", backgroundColor: "rgba(128, 128, 128, 0.15)", color: "" }, options);
    let lineNumbers;
    return function (editor) {
        highlight(editor);
        if (!lineNumbers) {
            lineNumbers = init(editor, opts);
        }
        const code = editor.textContent || "";
        const linesCount = code.split(/\r\n|\r|\n/).length + (code.endsWith('\r') || code.endsWith('\n') ? 0 : 1);
        let text = "";
        for (let i = 1; i < linesCount; i++) {
            text += `${i}\xa0\r\n`;
        }
		lineNumbers.style.setProperty("min-height", editor.scrollHeight + "px");
        lineNumbers.innerText = text;
    };
}
function init(editor, opts) {
    const css = getComputedStyle(editor);
    const wrap = document.createElement("div");
    wrap.className = opts.wrapClass + " token comment";
    wrap.style.position = "relative";
    wrap.style.setProperty("overflow-x", "auto");
    wrap.style.setProperty("overflow-y", "auto");
    const lineNumbers = document.createElement("div");
    lineNumbers.className = opts.class;
    wrap.appendChild(lineNumbers);
    // Add own styles
    lineNumbers.style.position = "absolute";
    lineNumbers.style.top = "0px";
    lineNumbers.style.left = "-0.8em";
    lineNumbers.style.bottom = "0px";
    lineNumbers.style.width = opts.width;
    lineNumbers.style.overflow = "unset";
    lineNumbers.style.backgroundColor = opts.backgroundColor;
    //lineNumbers.style.color = opts.color || css.color;
    lineNumbers.style.setProperty("mix-blend-mode", "difference");
    lineNumbers.style.setProperty("user-select", "none");
    // Copy editor styles
    lineNumbers.style.fontFamily = css.fontFamily;
    lineNumbers.style.fontSize = css.fontSize;
    lineNumbers.style.lineHeight = css.lineHeight;
    lineNumbers.style.paddingTop = css.paddingTop;
    lineNumbers.style.paddingLeft = css.paddingLeft;
    lineNumbers.style.borderTopLeftRadius = css.borderTopLeftRadius;
    lineNumbers.style.borderBottomLeftRadius = css.borderBottomLeftRadius;
    // Tweak editor styles
    editor.style.paddingLeft = `calc(${opts.width} + ${lineNumbers.style.paddingLeft})`;
    editor.style.whiteSpace = "pre";
    editor.style.setProperty("overflow-x", "unset");
    editor.style.setProperty("overflow-y", "unset");
    editor.style.setProperty("resize", "none");
    editor.style.setProperty("min-height", "100%");
    // Swap editor with a wrap
    editor.parentNode.insertBefore(wrap, editor);
    wrap.appendChild(editor);
    return lineNumbers;
}
