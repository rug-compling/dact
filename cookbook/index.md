---
layout: manual
title: Cookbook - Decaffeinated Alpino Corpus Tool
---

## Search a particular word

```
//node[@word='loopt']
```

## Proper name subjects

```
//node[@rel="su" and (@ntype="eigen" or @postag="SPEC(deeleigen)"]
```

(<a href="dact:/?filter=//node[@rel='su' and @pt='spec']">Run in Dact</a>)

Note that this query will only return single-word subjects, as in:

   Ajax speelde een verloren wedstrijd

We will not get examples such as

   Het Feyenood van van Hanegem boezemde elke tegenstander angst in

or

   FC Utrecht speelt zijn thuiswedstrijden in de Galgenwaard
   
or the combination, as in 

   Het FC Utrecht van Jan Wouters verloor drie wedstrijden op rij
      
If you also want to find these cases, then the query becomes a bit more complicated. At this point,
you may want to consider using macros.

We define the following macros:

single_name = """( @ntype = 'eigen' or @postag='SPEC(deeleigen)'  )"""

multi_name =  """( @cat='mwu' and node[@rel='mwp' and %single_name%] ) """

name =        """( %single_name% or %multi_name% )"""

name_phrase=  """( %name% or node[@rel="hd"  and %name%]  )"""

Then we can use the query:

//node[@rel="su" and %name_phrase%]


   

