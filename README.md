# CS224 Final Project &mdash; Render Denoising

## TODO

- **Factoring out the albedo**: if the albedo includes view-dependent
  info, it is different across samples in the sample pixel. Then,
  averaging each channe and factoring albedo back in changes the
  result. Only an issue for > 1 pixel per sample
