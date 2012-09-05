---
layout: manual
title: Cookbook - Decaffeinated Alpino Corpus Tool
---

## Find a particular word

    //node[@word='loopt']

This is case-sensitive.

If you want to find all inflectional variants of the verb 'lopen', do:

    //node[@lemma='lopen']

## Types of nodes

Nodes with daughter nodes can be distinguished by the "cat" attribute.

    //node[@cat]
    
Nodes without daughter nodes can be of two kinds: leaf nodes with a "word"
attribute, or indexed nodes. Leaf nodes of the first kind can be selected
with

    //node[@word]
    
Indexed nodes are nodes which are co-indexed with another node where the information
of that node is spelled out. Such nodes are leaf nodes, but have neither a "cat" nor
a "word" attribute.

    //node[@index and not(@word or @cat)]
    
To ensure that a node is not an indexed node, use

    //node[@word or @cat]

## Proper name subjects

    //node[@rel="su" and (@ntype="eigen" or @postag="SPEC(deeleigen)"]

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

## Surface order: comparison of attributes with numeric values

In the implementation of XPATH2 that is used in Dact, the comparison of numeric values may be
somewhat counter-intuitive. For instance, in order to find a prenominal modifiers in a noun-phrase,
the following query might be attemped:

    //node[../@cat="np" and @rel="mod" and @begin < ../node[@rel="hd"]/@begin]

However, this will not find any hits. Instead, we must explicitly convert the value of the attribute
@begin to numeric:

    //node[../@cat="np" and @rel="mod" and number(@begin) < ../node[@rel="hd"]/number(@begin)]

Typical attributes with numeric values are "begin", "end" and "index". The operators which require
the conversion include "<", ">" and "=".

Since the numeric conversion of the value of the attributes "begin", "end" and "index" is so common, we
have the following two macros:

    b = """number(@begin)"""
    e = """number(@end)"""
    i = """number(@index)"""

## Surface order: location of the head of a phrase

It is also very common to refer to the begin and end positions of the head of a phrase. We define
macros in two versions, depending on the notion of head that we wish to use. If the relation has to be "hd",
then we use "begin_of_hd" and "end_of_hd". If we also want to capture complementizers, coordinators etc,
we use "begin_of_head" and "end_of_head".

    headrel = """ ( @rel="hd" or @rel="cmp" or @rel="mwp" or @rel="crd" 
                 or @rel="rhd" or @rel="whd" or @rel="nucl" or @rel="dp" ) """

    begin_of_head = """ node[%headrel%]/%b% """
    end_of_head   = """ node[%headrel%]/%e% """

    begin_of_hd   = """ node[@rel="hd"]/%b% """
    end_of_hd     = """ node[@rel="hd"]/%e% """

## Topicalization, fronting, the "vorfeld"

In order to find constituents which are "topicalized" in root sentences (in other words, 
constituents which occupy the "vorfeld" position in a main clause, in other words, constituents
which are "fronted"), the following query
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

## Fronted comparative phrases 

This example is taken from Leonoor van der Beek, Gosse Bouma, Gertjan van Noord. Een brede computationele 
grammatica voor het Nederlands. Nederlandse Taalkunde, jaargang 7, 2002-4. 353--374.

Comparatives are often combined with a complement, as in:

> Lager dan ik dacht

The complement of the comparative is assigned the relation "obcomp". If the comparative ends up in the
vorfeld, its obcomp can be part of the vorfeld too, or it can be placed at the end of the sentence:

> Belangrijker nog dan de ligging was de uitmuntende bescherming van de graaf van de Champagne .

> Eerder staat een machine een half uur stil dan dure voorraden te produceren .

In order to find cases of the second type (extraposition of comparative complements out of topic position),
we used the following query:

    //node[@cat="smain" and 
           node[
                node[@rel="obcomp"]/%e% > ../node[@rel="hd"]/%b%
                ]/%b% = %b%
          ]

Given the discussion of vorfeld in the previous section, the query can be improved to:

    //node[ %vorfeld% and 
            node[@rel="obcomp" and 
                 %b% > ../../node[@rel="hd"]/%b%
                 ]
          ]


Note that this construction is rather infrequent (some linguists even claim it to be ungrammatical). In some
treebanks, you may not get any hits. In the Lassy Large treebank, many hits are mis-parses. But some genuine examples 
are found!

## Antecedents of co-indexed nodes

Suppose we want to find all nouns which can be used as the direct object of the verb "drinken".
We might try

    //node[@rel="obj1" and ../node[@rel="hd" and @lemma="drinken"]]
    
This will give all noun-phrases. In case the noun phrases are lexical, we are done. Otherwise,
we want to select the head daughter:



    //node[ (  ( @rel="obj1" and @word and ../node[@rel="hd" and @lemma="drinken"])
            or ( @rel="hd" and ../@rel="obj1" and ../../node[@rel="hd" and @lemma="drinken"]] )
            )]
            
This will give good examples, but it will miss to find for instance:

> " Wat drinkt de Belg ? " <a href="7.svg">(SVG)</a>

The reason is, that the direct object of  "drinken" in this case is an index node. In order to get at the
information associated with that node, we need to find the antecedent of the index node. This is a node with
the same value for the attribute "index".

If the antecedent is lexical, the query is:

    //node[(@cat or @word) and @index = //node[@rel="obj1" and ../node[@rel="hd" and @lemma="drinken"]]/@index]
           
If the antecedent is not lexical, we reason from its head:

    //node[@rel="hd" and ../@index = //node[@rel="obj1" and ../node[@rel="hd" and @lemma="drinken"]]/@index]

The various queries could be combined to find all relevant cases in one go, but it would probably be easier
to use macros for that. 
    
## Secondary object passives with "krijgen"

This example is taken from Valia Kordoni and Gertjan van Noord. Passives in Germanic
Languages: the case of Dutch and German. In: Groninger Arbeiten zur
Germanistischen Linguistik (GAGL). Volume 49. pp 77-96. December 2009
(appeared in 2010). 

A sentence such as 

>  hij krijgt een microfoon onder de neus geduwd

is analysed such that "hij" is both the subject of the verb "krijgt" as well as 
the secondary object of the verb "geduwd".  In order to find examples of this construction, the following
query can be used:

    //node[ node[@rel="hd" and @lemma="krijgen"] and
            node[@rel="su"]/%i% = node[@rel="vc"]/node[@rel="obj2"]/%i% 
          ]
          
Note that this identifies the dominating node of this construction. If, on the other hand, we are interested to
identify the various verbs which occur in this construction ("geduwd" in the example), we need to define the query
in a somewhat different way:

    //node[@rel="hd" and @pt="ww" and 
           ../../node[@rel="hd" and @lemma="krijgen"] and 
           ../node[@rel="obj2"]/%i% = ../../node[@rel="su"]/%i%
           ] 
           
Counting the attribute "word" of the matching nodes, might produce something like:

    ingeplant       5
    gedaan	        3
    geslingerd	    3
    opgelegd	    3
    voorgeschoteld	3
    aangenaaid	    2
    geworpen	    2
    uitgekeerd	    2
    aangerekend	    1
    betaald	        1



