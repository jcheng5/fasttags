#include <Rcpp.h>
using namespace Rcpp;

// This is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp
// function (or via the Source button on the editor toolbar). Learn
// more about Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//   http://gallery.rcpp.org/
//

void write_any(std::ostream& out, const RObject& x, int indent);
void write_list(std::ostream& out, const List& list, int indent);
void write_tag(std::ostream& out, const List& tag, int indent);
void write_text(std::ostream& out, const CharacterVector& text, int indent);
void write_html(std::ostream& out, const CharacterVector& html, int indent);

void _write_indent(std::ostream& out, int indent);
void _write_eol(std::ostream& out, int indent);

void _write_escaped(std::ostream& out, const std::string& str) {
  for (std::string::const_iterator it = str.begin();
    it != str.end();
    it++) {

    switch (*it) {
    case '&': out << "&amp;"; break;
    case '<': out << "&lt;";  break;
    case '>': out << "&gt;";  break;

    default: out << *it;
    }
  }
}
void _write_attrib_value(std::ostream& out, const std::string& str) {
  for (std::string::const_iterator it = str.begin();
    it != str.end();
    it++) {

    switch (*it) {
    case '&':  out << "&amp;";  break;
    case '<':  out << "&lt;";   break;
    case '>':  out << "&gt;";   break;
    case '\'': out << "&#39;";  break;
    case '"':  out << "&quot;"; break;
    case '\r': out << "&#13;";  break;
    case '\n': out << "&#10;";  break;

    default: out << *it;
    }
  }
}

//' Fast but sloppy HTML rendering
//'
//' Renders \pkg{htmltools} tag graphs into HTML. This is missing several
//' features of \link[htmltools]{renderTags}, including but not necessarily
//' limited to: singleton, head, and dependency handling; converting objects
//' to tags using \link[htmltools]{as.tags}; combining attributes with the
//' same name by space-delimiting them; and possibly others.
//'
//' @param x An htmltools tag object.
//' @return An HTML string (marked as if returned from \link[htmltools]{HTML}).
//' @useDynLib fasttags
//' @importFrom Rcpp evalCpp
//' @export
// [[Rcpp::export]]
CharacterVector fastrender(const RObject& x) {
  CharacterVector output;

  std::ostringstream out;
  write_any(out, x, 0);

  output.push_back(out.str());

  CharacterVector classes;
  classes.push_back("html");
  classes.push_back("character");
  output.attr("class") = classes;
  output.attr("html") = true;
  return output;
}

void write_any(std::ostream& out, const RObject& x, int indent) {
  if (R_NilValue == x) {
    return;
  }

  if (R_NilValue != x.attr("class")) {
    CharacterVector klass = Rcpp::as<CharacterVector>(x.attr("class"));
    for (CharacterVector::iterator it = klass.begin();
      it != klass.end();
      it++) {

      if (*it == "html") {
        write_html(out, Rcpp::as<CharacterVector>(x), indent);
        return;
      } else if (*it == "shiny.tag") {
        write_tag(out, Rcpp::as<List>(x), indent);
        return;
      }
    }
  }

  if (TYPEOF(x) == STRSXP) {
    write_text(out, Rcpp::as<CharacterVector>(x), indent);
  } else if (TYPEOF(x) == VECSXP) {
    write_list(out, Rcpp::as<List>(x), indent);
  } else {
    // TODO: Convert using as.tags?
    write_text(out, Rcpp::as<CharacterVector>(x), indent);
  }
}

void write_list(std::ostream& out, const List& list, int indent) {
  for (List::const_iterator it = list.begin();
    it != list.end();
    it++) {

    write_any(out, *it, indent);
  }
}

void write_tag(std::ostream& out, const List& tag, int indent) {
  std::string name = Rcpp::as<std::string>(tag("name"));
  Rcpp::List attribs = Rcpp::as<Rcpp::List>(tag("attribs"));
  Rcpp::List children = Rcpp::as<Rcpp::List>(tag("children"));

  // start tag
  _write_indent(out, indent);
  out << "<" << name;
  for (size_t i = 0; i < attribs.length(); i++) {
    if (attribs[i] == R_NilValue) {
      continue;
    }
    out << " " << Rcpp::as<CharacterVector>(attribs.names())(i);
    CharacterVector attrib_value = Rcpp::as<CharacterVector>(attribs[i]);
    if (attrib_value.length() == 1 && !is_na(attrib_value)[0]) {
      out << "=\"";
      _write_attrib_value(out, Rcpp::as<std::string>(attrib_value[0]));
      out << "\"";
    }
  }
  out << ">";

  bool simple_text_child = false;
  if (children.length() == 1) {
    RObject first_child = children[0];
    if (TYPEOF(first_child) != VECSXP && Rf_length(first_child) == 1) {
      simple_text_child = true;
    }
  }
  if (simple_text_child) {
    write_any(out, children[0], -1);
  } else if (children.length() > 0) {
    _write_eol(out, indent);
    write_any(out, children, indent >= 0 ? indent + 1 : indent);
    _write_indent(out, indent);
  }

  // end tag
  out << "</" << name << ">";
  _write_eol(out, indent);
}

void write_text(std::ostream& out, const CharacterVector& text, int indent) {
  for (CharacterVector::const_iterator it = text.begin();
    it != text.end();
    it++) {
    _write_indent(out, indent);
    _write_escaped(out, Rcpp::as<std::string>(*it));
    _write_eol(out, indent);
  }
}

void write_html(std::ostream& out, const CharacterVector& html, int indent) {
  for (CharacterVector::const_iterator it = html.begin();
    it != html.end();
    it++) {

    _write_indent(out, indent);
    out << Rcpp::as<std::string>(*it);
    _write_eol(out, indent);
  }
}

void _write_indent(std::ostream& out, int indent) {
  for (int i = 0; i < indent; i++) {
    out << "  ";
  }
}

void _write_eol(std::ostream& out, int indent) {
  if (indent >= 0)
    out << "\n";
}

// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically
// run after the compilation.
//

/*** R
data <- readRDS("../shinyTags.rds")
system.time({
  #rendered <- fastrender(tags$div(tags$span("Hi")))
  rendered <- fastrender(data)
})
*/
