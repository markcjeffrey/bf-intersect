Bloom filter null-intersection tests
====================================
This code determines the rate of false set-overlap of different
Bloom-filter-based null-intersection tests. Find more details in this
[SPAA 2011 paper](https://doi.org/10.1145/1989493.1989551) or
[MASc thesis](https://tspace.library.utoronto.ca/bitstream/1807/30640/1/Jeffrey_Mark_C_201111_MASc_thesis.pdf).
The code was used to generate data for Figures 3.2 and 4.2 of the thesis. We
use "signature" and "Bloom filter" interchangeably, as is common in the
computer architecture community.

If you use this software in your research, I request that you reference the
paper in any resulting publications, and that you send me a citation of your
work.
```bib
@inproceedings{jeffrey:spaa11:bf-intersection,
 author = {Jeffrey, Mark C. and Steffan, J. Gregory},
 title = {Understanding {Bloom} filter intersection for lazy address-set disambiguation},
 booktitle = {Proc. of the 23rd ACM Symposium on Parallelism in Algorithms and Architectures (SPAA)},
 year = {2011},
 month = {June},
}
```
If you use the "Batch of Bloom filters" or "Batched signature" design in your
research, I request that you reference the thesis in any resulting
publications.
```bib
@mastersthesis{jeffrey:thesis11:bf-intersection,
 author = {Jeffrey, Mark C.},
 title = {Understanding and Improving {Bloom} Filter Configuration for Lazy Address-Set Disambiguation},
 year = {2011},
 school = {University of Toronto},
}
```
