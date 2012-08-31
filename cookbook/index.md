---
layout: manual
title: Cookbook - Decaffeinated Alpino Corpus Tool
---

## Search a particular word

    //node[@word='loopt']


## Proper name subjects

    //node[@rel="su" and (@ntype="eigen" or @postag="SPEC(deeleigen)"]

(<a href="dact:/?filter=//node[@rel='su' and @pt='spec']">Run in Dact</a>)

Note that this query will only return single-word subjects, as in:

>   Ajax speelde een verloren wedstrijd <a href="1.svg">(SVG)</a>

We will not get examples such as

>   Het Feyenoord van van Hanegem boezemde elke tegenstander angst in <a href="2.svg">(SVG)</a>

or

>   FC Utrecht speelt zijn thuiswedstrijden in de Galgenwaard <a href="3.svg">(SVG)</a>
   
or the combination, as in 

>   Het FC Utrecht van Jan Wouters verloor drie wedstrijden op rij <a href="4.svg">(SVG)</a>
      
If you also want to find these cases, then the query becomes a bit more complicated. At this point,
you may want to consider using macros.

We define the following macros:

    single_name = """( @ntype = 'eigen' or @postag='SPEC(deeleigen)'  )"""
    
    multi_name =  """( @cat='mwu' and node[@rel='mwp' and %single_name%] ) """
    
    name =        """( %single_name% or %multi_name% )"""

    name_phrase=  """( %name% or node[@rel="hd"  and %name%]  )"""

Then we can use the query:

    //node[@rel="su" and %name_phrase%]

## Topicalization

Since the numeric conversion of the value of the attributes "begin" and "end" is so common, we
have the following two macros:

    b = """number(@begin)"""
    e = """number(@end)"""

It is also very common to refer to the begin and end positions of the head of a phrase. We define
two versions, depending on the notion of head that we wish to use. If the relation has to be "hd",
then we use "begin_of_hd" and "end_of_hd". If we also want to capture complementizers, coordinators etc,
we use "begin_of_head" and "end_of_head".

    headrel = """ ( @rel="hd" or @rel="cmp" or @rel="mwp" or @rel="crd" 
                 or @rel="rhd" or @rel="whd" or @rel="nucl" or @rel="dp" ) """

    begin_of_head = """ node[%headrel%]/%b% """
    end_of_head   = """ node[%headrel%]/%e% """

    begin_of_hd   = """ node[@rel="hd"]/%b% """
    end_of_hd     = """ node[@rel="hd"]/%e% """

In order to find constituents which are "topicalized" in root sentences (in other words, 
constituents which occupy the "vorfeld" position in a main clause), the following query
may be proposed. In this query, we want to find elements of a main clause which start at the same 
position as the main clause as a whole:

    //node[../@cat="smain" and %b% = ../%b% ]

The query will find many genuine examples of topicalized constituents, but it will not find *all*
relevant cases. This is so, because a topicalized constituent is not always a child of a
main clause. It can be embedded somewhere deeper in the sentence, as in:

>   Wat denk je dat hij zei <a href="5.svg">(SVG)</a>

In order to catch such cases as well, the query will be formulated in a more complicated
manner as follows. We will define vorfeld as a 'maximal' constituent which precedes the head of a
main clause. The head of a main clause normally is the finite verb. By 'maximal' we mean that we 
do not want to find sub-parts of a vorfeld constituent. Consider:

>   De man met de zaag slaapt <a href="6.svg">(SVG)</a>

In this example, the constituent 'met de zaag' also precedes the finite verb, but it is not itself the vorfeld
constituent, but only a part of it.

The following set of macros define vorfeld constituents. We first define 'precedes_head_of_smain'. A constituent
then is a vorfeld if it precedes the head of an smain clause, and it is not part of a constituent which precedes
the head of an smain clause.

    precedes_head_of_smain = """
    (  ancestor::node[@cat="smain"]/
                 node[@rel="hd"]/%b% 
               > %begin_of_head% 
       or 
       ancestor::node[@cat="smain"]/
                 node[@rel="hd"]/%b% 
               > %b% and @pos
    ) """

    vorfeld = """
    %precedes_head_of_smain% and not (ancestor::node[%precedes_head_of_smain%]) """


To find topicalized indirect objects, do:

    //node[@rel="obj2" and %vorfeld%]

