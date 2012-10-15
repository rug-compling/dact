---
layout: manual
title: Cookbook - Decaffeinated Alpino Corpus Tool
---

* Table of Contents
{:toc}

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

## Sentence types

In some cases, this is straightforward. In other cases, this is almost impossible given the annotation scheme.

### Finite clauses

To find main clauses:

    //node[@cat="smain"]
    
Finite subordinate clauses:

    //node[@cat="cp" and node[@rel="body" and @cat="ssub"]]

In order to determine the proportion of verb-second and verb-final finite clauses, wen pose
the following query:

    //node[@cat="smain" or @cat="ssub"]
    
and count the frequency of the "cat" attribute in the statistics widget



<table align="right">
<tr>
    <td>smain</td>    <td>58608</td>    <td>70.8%</td> 
</tr>
<tr>
    <td>ssub</td>     <td>24211</td>    <td>29.2% </td>
</tr>
</table>



### Relative clauses

Finite relative clauses:

    //node[@cat="rel" and node[@rel="body" and @cat="ssub"]]

Free relatives are marked with a separate value for the "cat" attribute:

    //node[@cat="whrel"]

We can find the various roles that such free relatives play by counting the attribute "rel" in
the statistics window:

    mod   469
    obj1  244
    su	  169
    predc  61
    dp	   42
    cnj	   41
    sat	   35
    --	   29
    body   13
    nucl    9
    tag	    3

### Questions

To find direct wh-questions:

    //node[@cat="whq"]
    
To find indirect questions:

    //node[@cat="whsub"]
    
The following query identifies WH-elements which are associated with an embedded finite verb (non-local 
dependency), as in:

> Wat denk je dat er gaat gebeuren ?

> Wie denken ze wel dat ze zijn bij Urbanbite ? 

> ,, Bedoel je wat ik denk dat je bedoelt ? '' vroeg Bush nors . 

> " Hoe zeg je dat die groep heet , Solex ? '' , vraagt Bouwens . 

    //node[@rel="whd" and %i% = //node[../../@cat="cp" and ../../@rel="vc"]/%i%]
    
### Verb-initial clauses

The following query:

    //node[@cat="sv1"]
    
identifies imparatives as well as yes/no-questions and indeed some further constructions. If we want to find 
imparatives only, the following query is useful:

    //node[@cat="sv1" and not(node[@rel="su"])]

However, this will also identify "topic-drop" sentences in which the subject is dropped, as in:

> Wordt behandeld in de volgende sectie.

> Is ook regelmatig te zien in stadstuinen.

> Vormen samen het Duitse taalgebied in België.

Moreover, we rule out imparatives which actually do have a subject, as in:

> Bewaart u hem bij uw reispapieren .

> Houdt u zich aan de gebruiksaanwijzing van elektrische toestellen .

Unless we rely on the presence of a question mark, the distinction between the question and the imparative
is not encoded in the annotation.

Some of topic-drop cases can be ruled out if we require that the finite verb is not plural:

    //node[@cat="sv1" and not(node[@rel="su"]) and not(node[@rel="hd" and @pvagr="mv"])]

We may be tempted to rule out cases with "@pvagr='met-t'" as well, but that will rule out more old-fashioned
imparatives such as:

> Eert uw vader en uw moeder

Note that in this example, the annotation guidelines regrettably do not impose a distinction between the imparative reading and
the topic-drop reading. 

We identify many yes/no-questions with the following query:

    //node[@cat="sv1" and node[@rel="su"] and 
           not(@rel="body" or @rel="tag" or @rel="sat") and 
           not(@rel="cnj" and (../@rel="body" or ../@rel="tag" or ../@rel="sat"))]
    
Some false hits involve conjunctions of the following type:

> Ditvoorst beschikte overduidelijk over het eerste talent , maar kwam de andere twee tekort .

The clause "kwam aan de andere twee tekort" is specified as cat=sv1. We can rule out such cases if the query is
extended as follows. Note that we do not rule out all coordinations, but only those coordinations where one of
the conjuncts is an smain clause.

    //node[@cat="sv1" and node[@rel="su"] and 
           not(@rel="body" or @rel="tag" or @rel="sat") and 
           not(@rel="cnj" and (../@rel="body" or ../@rel="tag" or ../@rel="sat")) and
           not(@rel="cnj" and ../node[@rel="cnj" and @cat="smain"])]
           
This query still finds quite a few cases of SV1 which are not yes/no-questions. The most practical solution may
simply be to require that the clause is followed by a question mark, and is not part of a WH-question:

    //node[@cat="sv1" and 
           not(@rel="body") and
           %e% < //node[@word="?"]/%e%]

### Verbs which select an indirect question

For Tristan. If you are interested in all verbs which can be used with an indirect WH-question, you can
use the following query:

    //node[@rel="hd" and 
           @pt="ww" and
           ../node[@rel="vc" and @cat="whsub"]
          ]

For the SONAR3 sub-part of the Lassy Large corpus, the most frequent verb lemmas are:

    weten      112588
    zien        29686
    vragen      24283
    zeggen      14398
    vertellen   12811
    
Indirect yes/no-questions can be found with the query:

    //node[@rel="hd" and
           @pt="ww" and
           ../node[@rel="vc" and 
                   node[@rel="cmp" and
                        @lemma="of"
                       ]
                   ]
          ]
          
          

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

## Surface order

### Comparison of attributes with numeric values

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

### Bigrams, trigrams

To find occurrences of a bigram, e.g. "wel niet", use:

    //node[@lemma="wel" and 
           %e%=//node[@lemma="niet"]/%b%]

This technique can be extended to longer N-grams, e.g. for "wel of niet" use:

    //node[@lemma="wel" and 
           %e%=//node[@lemma="of" and 
                      %e%=//node[@lemma="niet"]/%b%
                     ]/%b%
          ]

These examples can be improved if we have access to parameterized macros.

### Location of the head of a phrase

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

## Minimal dominating node

In some cases you want to find a node with particular properties, and that node has to be
the "minimal" node in the sense that it does not dominate a node with such properties. For example,
in Catherine Lai and Steven Bird, Querying and updating treebanks: A critical survey and requirements analysis,
in: Proceedings of the Australasian Language Technology Workshop, pages 139-146, 2004, an example of such
a query is given. We paraphrase the query as: identify the minimal node with dominates a NP PP sequence.

We first define "dominates_np_pp_seq" and then use that definition in the solution for this problem.
A node dominates a NP PP sequence if it dominates an NP and a PP, where the NP directly precedes the PP:

    dominates_np_pp_seq = """ .//node[@cat="np"]/%e% = .//node[@cat="pp"]/%b% """
  
The minimal node is then identified by the following query:
  
    //node[%dominates_np_pp_seq% and
            not(node[%dominates_np_pp_seq%])]
            
In this particular case, it suffices to inspect only potential daughter nodes. In other cases, things may be
somewhat more complicated. For instance, if we want to find minimal NP's, we use the ".//" axis which identifies
all nodes dominated by the current node:

    //node[@cat="np" and not(.//node[@cat="np"])]
    
Rather than "minimal" nodes, we might also want to identify "maximal" nodes. Such an example occurs in the next
section on topicalization, where the "ancestor::" axis is helpful.

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

Given the discussion in section [vorfeld](#topicalization-fronting-the-vorfeld) and 
section [nachfeld](#extraposition-the-nachfeld) in other sections of the cookbook, we may also write:

    //node[%vorfeld% and 
            node[@rel="obcomp" and 
                 %nachfeld%
                ]
          ]
          
Note that both queries are not equivalent.

Also note that this construction is rather infrequent (some linguists even claim it to be ungrammatical). In some
treebanks, you may not get any hits. In the Lassy Large treebank, many hits are mis-parses. But some genuine examples 
are found!

> Eerder zullen zij de wetten der fysica beheersen dan die van het fatsoen . 

> Liever wilde hij zijn Führer door overreding tot inkeer brengen , dan zijn toevlucht nemen tot een moordaanslag . 

> Zozeer heeft Dominique de Villepin aan gezag ingeboet dat ook in zijn eigen centrumrechtse UMP-partij hoe langer hoe meer parlementsleden om zijn aftreden begonnen te roepen . 

> slechter kan de Nederlandse bolletjesslikker het niet treffen dan in de cellencomplexen van de politiebureaus in Paramaribo , zoals bij bureau Keizerstraat 

> Wat anders kan hij doen dan zijn schouders ophalen , zwijgen of , beter nog , in lachen uitbarsten ? 

> Klassieker kan een beurscorrectie niet verlopen dan de afgelopen dagen . 

## Unbounded dependencies

The following query will find WH-questions in which the WH-element is a dependent of an embedded finite verb,
the prototypical case of a possibly unbounded dependency. 

    //node[@rel="whd" and 
           ../@cat="whq" and
           %i% = //node[../@cat="ssub"]/%i%
           ]
           
If you want to identify the verbs which "license" such a construction (the term "bridge verbs" is sometimes
used for such cases), the query is:

    //node[@rel="hd" and 
           @pt="ww" and 
           ../node/node[@cat="ssub" and 
                        node[%i%=//node[@rel="whd" and 
                                        ../@cat="whq"]/%i%  
                            ]
                        ] 
           ]

## Verb Clusters

In the Lassy corpora, verb clusters are typically not represented as a single node. For instance, the following
sentence from Lassy:

> De reactie was zwakker toen de proefpersonen het geld bij de belastingen zagen terechtkomen . <a href="dpc-ind-001634-nl-sen.p.9.s.4.svg">(SVG)</a>

In order to identify such constructions nonetheless, we can use the following macro definition to identify 
the complement verb which is part of the verb cluster together with its dependents. In the example above, this query
identifies the VP headed by "terechtkomen".

    verbcluster = """( @rel="vc" and 
                       (@cat="ti" or @cat="inf" or @cat="ppart") and 
                       node/%b% < ../node[@rel="hd" and @pt="ww"]/%b%
                     )"""
                     
The "verbcluster" macro is useful to find, for instance, occurrences of the infamous cross-serial dependency 
construction (also known as AcI - Accusativus cum infinitivo). In that construction, the direct object of a verb 
such as "zien", "horen" or "laten" is identified as the subject of the embedded infinite verb phrase. The 
macro "cross_serial_verbcluster" identifies such verb phrases:

    cross_serial_verbcluster = """ ( 
            //node[%verbcluster% and @cat="inf" and 
                   ../node[@rel="obj1"]/%i% = node[@rel="su"]/%i%
                  ]                ) """
                    
To find out which governing verbs occur in a cross serial dependeny construction, we can then do:

    //node[@rel="hd" and ../node[%cross_serial_verbcluster%]]
                        
Counting the frequency of the value of the "lemma" attribute gives:
                    
    laten    199
    doen	  42
    zien	  32
    hebben	   7
    vinden	   3
    helpen	   2
    horen	   2
    voelen	   1                    

## Using quantifiers in XPATH2

In XPATH2, quantified queries have been introduced which provide for additional possibilities. 
As an example of the potential use of quantified expressions, consider
the query in which we want to identify a NP which contains a VC complement
(infinite VP complement), in such a way that there is a noun which is preceded by
the head of that NP, and which precedes the VC complement. 

In this example:

> Ik heb de hoop opgegeven hem ooit terug te zien

the VP "hem ooit terug te zien" is a VC complement of "hoop". Is it the case that such a VC
complement is always associated with the most "recent" noun? Such a query can be formulated as
follows:

    //node[@cat="np" and 
          ( some $tussen in //node[@pos="noun"] 
           satisfies (   $tussen/%b% 
                       < node[@rel="vc"]/%b% and 
                         $tussen/%e% 
                       > node[@rel="hd"]/%e%
                     )
         )]
         
As it turns out, such cases occur regularly, as in:

> Verschillende pogingen van de zusjes om elkaar terug te vinden worden uiteindelijk door de oorlog gefrustreerd .

## Extraposition, the "nachfeld"

Constituents which are placed to the right of the head of a VP or a subordinate clause are
often said to be "extraposed", or to occupy the "nachfeld" position. The following set of
macro definitions are provided to identify such constituents:

    vp = """ (@cat="inf" or @cat="ti" or @cat="ssub" or @cat="oti" or @cat="ppart") """

    nachfeld = """
    ( not(%verbcluster%) and
      not(@rel="hd" and parent::node[%verbcluster%]) and
      ( some $v in ( ancestor::node[%vp%]/node[@rel="hd"] ) 
        satisfies
  	       (
                (  $v/%b% < %begin_of_head%
                or (  $v/%b%  < %b% and @word )
                )
                and not( parent::node[$v/%b% < %begin_of_head%] )
  	       )		
      )
    )"""

With these macros in place, we can find extraposition of PP's out of NP, as in cases like

>  Lange tijd is de stad tevens het belangrijkste internationale centrum geweest van cultuur, kennis en geleerdheid

Here, the PP "van cultuur, kennis en geleerdheid" is a dependent of "centrum", but it is placed to the
right of the main verb. The following query uses the "nachfeld" macro to find extraposition of PP out of NP:

    //node[%nachfeld% and @cat="pp" and ../node[@rel="hd" and @pt="n"]]

Note that in root sentences in which the finite verb occupies the first or second position, and in which there
are nu further verbs, no extraposed phrases are found with the previous query. For instance, in:

> Ik ben bang voor muizen

the clause "voor muizen" is not extraposed according to our query. In this case, there are two corresponding
subordinate sentences, one of which contains an instance of extraposition.

> Omdat ik bang voor muizen ben

> Omdat ik bang ben voor muizen

A somewhat counter-intuitive result is obtained for root sentences in which there is only a corresponding
subordinate sentence with extraposition, as in:

> Hij is langer dan ik dacht

> Omdat hij langer is dan ik dacht

> *Omdat hij langer dan ik dacht is

So even if it appears to be the case that the main clause also contains an extraposed phrase, the query above will not
find it.


## Antecedents of co-indexed nodes; find all dependents of a particular type for a particular word

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

Recall we have the following macro to refer to the index of a node:

    i = """number(@index)"""
    
If the antecedent is lexical, the query is:

    //node[(@cat or @word) and %i% = //node[@rel="obj1" and ../node[@rel="hd" and @lemma="drinken"]]/%i%]
           
If the antecedent is not lexical, we reason from its head:

    //node[@rel="hd" and ../%i% = //node[@rel="obj1" and ../node[@rel="hd" and @lemma="drinken"]]/%i%]

The various queries could be combined to find all relevant cases in one go, but it is much easier
to use macros for that:


    obj1_drinken_lexical = """
    ( @rel="obj1" and 
      @word and 
      ../node[@rel="hd" and 
              @lemma="drinken"]
    )"""

    obj1_drinken_phrase = """
    ( @rel="hd" and 
      ../@rel="obj1" and 
      ../../node[@rel="hd" and 
                 @lemma="drinken"]
    )"""

    obj1_drinken_lexical_nonlocal = """
    ( (@cat or @word) and 
      %i% = //node[@rel="obj1" and 
                   ../node[@rel="hd" and 
                           @lemma="drinken"]]/%i%
    )"""
    
    obj1_drinken_phrase_nonlocal = """
    ( @rel="hd" and 
      ../%i% = //node[@rel="obj1" and 
                      ../node[@rel="hd" and 
                      @lemma="drinken"]]/%i%
    )"""
    
    obj1_drinken = """
    (  %obj1_drinken_lexical%
    or %obj1_drinken_phrase%
    or %obj1_drinken_lexical_nonlocal%
    or %obj1_drinken_phrase_nonlocal%
    )
    """

This example also illustrate that parameterized macros would be a nice extension for DACT. For efficiency,
take into account [section Query pipelines](#query-pipelines)

## Query pipelines

As a special extension to XPATH, DACT supports the use of a query pipeline. A query pipeline is a sequence
of queries separated by the special string "+|+". Only documents for which the N-th query succeeds are input for the
N+1-th query. An important motivation for query pipelines is efficiency. Consider the query:

    //node[%obj1_drinken%]
    
on a large corpus, this query takes a very long time. Yet, from the definition of the query it is clear that
the only documents which might contain hits contain a node with @lemma="drinken". A query 

    //node[@lemma="drinken"]

is very efficient because of the underlying index-mechanism of the database. For the compliated query the indexing
scheme is not powerful enough. Query pipelines come to the rescue:

    //node[@lemma='drinken'] +|+ //node[%obj1_drinken%]

The complicated query needs to be evaluated only on the documents which contain the verb "drinken". This saves
hours of computation time.

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

## Agreement mismatches and other surprises

It is sometimes interesting to search for constructions which you might think do not occur. The following query
finds plural heads which are combined with a determiner which usually combines only with singular heads:

    //node[node[@getal="mv" and @rel="hd"] and 
           node[@rel="det" and (@lemma="het" or 
                                @lemma="een" or 
                                @lemma="dit" or 
                                @lemma="dat")
                ]
          ]

Some of these identify sentences with grammatical errors, or annotation mistakes. 

Relative pronouns which are not part of a relative clause:

    //node[@vwtype="betr" and not(@rel="rhd" or ../@rel="rhd")]
    
Relative clauses without a relative pronoun:

     //node[@rel="rhd" and @word and 
            not(@vwtype="betr" or 
                @vwtype="vb" or
                @pt="bw")
           ]
           
Subjects which do not occur in the context of a verb:

    //node[node[@rel="hd" and @pt and not(@pt="ww")] and
           node[@rel="su"]
           ]



