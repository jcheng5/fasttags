# fasttags

A package that renders [htmltools](https://cran.r-project.org/web/packages/htmltools/index.html) tag graphs much, much faster than the normal methods.

Just pass your tag object to the `fastrender` function; it's approximately equivalent to calling `htmltools::doRenderTags` (which is itself a lower-level, less featureful version of `htmltools::renderTags`).

```r
> bigtags <- tags$select(name="foo", lapply(1:5e4, function(i) {tags$option(value=i, i)}))
> system.time(renderTags(bigtags))
   user  system elapsed 
 27.242   0.195  27.509 
> system.time(doRenderTags(bigtags))
   user  system elapsed 
 11.292   0.141  11.590 
> system.time(fastrender(bigtags))
   user  system elapsed 
  0.155   0.004   0.161 
```

## Limitations

fastrender is not, today, a drop-in replacement for `htmltools::renderTags`. The latter has a number of features that are not currently supported in this package:

* `tags$head()` is not respected
* `tags$singleton()` is not respected
* HTML dependency objects are thrown away
* Unrecognized objects in the graph are not coerced to tags using `as.tags`
* Multiple attributes with the same name are not combined into one space-delimited attributes (e.g. `tags$button(class="btn", class="btn-default")`)

As a result, this package should only really be used for simple-yet-large object graphs. htmlwidgets in particular will probably not work.

## License

GPL >=2
