/* Tiny, dependency-free C++ tokenizer for display highlighting.
   Returns HTML with <span class="tk-*"> wrappers. Class names map to CSS
   variables so the same markup recolors itself in light/dark.  */
(function () {
  const KEYWORDS = new Set("alignas alignof and asm auto bool break case catch char char8_t char16_t char32_t class concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept not nullptr operator or private protected public register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while".split(" "));
  const TYPES = new Set("std string vector size_t ostream istream ostringstream Node int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t uint64_t".split(" "));

  function esc(s) {
    return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
  }

  function highlight(src) {
    let out = "";
    let i = 0;
    const n = src.length;
    while (i < n) {
      const c = src[i];

      // line comment
      if (c === "/" && src[i + 1] === "/") {
        let j = i;
        while (j < n && src[j] !== "\n") j++;
        out += `<span class="tk-com">${esc(src.slice(i, j))}</span>`;
        i = j;
        continue;
      }
      // block comment
      if (c === "/" && src[i + 1] === "*") {
        let j = i + 2;
        while (j < n && !(src[j] === "*" && src[j + 1] === "/")) j++;
        j = Math.min(n, j + 2);
        out += `<span class="tk-com">${esc(src.slice(i, j))}</span>`;
        i = j;
        continue;
      }
      // preprocessor
      if (c === "#" && (i === 0 || src[i - 1] === "\n")) {
        let j = i;
        while (j < n && src[j] !== "\n") j++;
        // color the directive word, keep includes readable
        out += `<span class="tk-pre">${esc(src.slice(i, j))}</span>`;
        i = j;
        continue;
      }
      // string / char
      if (c === '"' || c === "'") {
        let j = i + 1;
        while (j < n && src[j] !== c) {
          if (src[j] === "\\") j++;
          j++;
        }
        j = Math.min(n, j + 1);
        out += `<span class="tk-str">${esc(src.slice(i, j))}</span>`;
        i = j;
        continue;
      }
      // number
      if (/[0-9]/.test(c) || (c === "." && /[0-9]/.test(src[i + 1] || ""))) {
        let j = i;
        while (j < n && /[0-9a-fA-FxX._]/.test(src[j])) j++;
        out += `<span class="tk-num">${esc(src.slice(i, j))}</span>`;
        i = j;
        continue;
      }
      // identifier / keyword
      if (/[A-Za-z_]/.test(c)) {
        let j = i;
        while (j < n && /[A-Za-z0-9_]/.test(src[j])) j++;
        const word = src.slice(i, j);
        // function call? next non-space char is (
        let k = j;
        while (k < n && src[k] === " ") k++;
        let cls = "";
        if (KEYWORDS.has(word)) cls = "tk-kw";
        else if (TYPES.has(word)) cls = "tk-type";
        else if (src[k] === "(") cls = "tk-fn";
        if (cls) out += `<span class="${cls}">${esc(word)}</span>`;
        else out += esc(word);
        i = j;
        continue;
      }
      // punctuation
      if (/[{}()\[\];,.<>:+\-*/%=&|!?~^]/.test(c)) {
        out += `<span class="tk-pun">${esc(c)}</span>`;
        i++;
        continue;
      }
      out += esc(c);
      i++;
    }
    return out;
  }

  window.cppHighlight = highlight;
})();
