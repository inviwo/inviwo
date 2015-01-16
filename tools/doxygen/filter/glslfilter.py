

<!DOCTYPE html>
<html>
  <head prefix="og: http://ogp.me/ns# fb: http://ogp.me/ns/fb# githubog: http://ogp.me/ns/fb/githubog#">
    <meta charset='utf-8'>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <title>glslAdditions/doxygen/glslfilter.py at master · numb3r23/glslAdditions · GitHub</title>
    <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub" />
    <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub" />
    <link rel="apple-touch-icon" sizes="57x57" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="114x114" href="/apple-touch-icon-114.png" />
    <link rel="apple-touch-icon" sizes="72x72" href="/apple-touch-icon-144.png" />
    <link rel="apple-touch-icon" sizes="144x144" href="/apple-touch-icon-144.png" />
    <link rel="logo" type="image/svg" href="https://github-media-downloads.s3.amazonaws.com/github-logo.svg" />
    <meta property="og:image" content="https://a248.e.akamai.net/assets.github.com/images/modules/logos_page/Octocat.png">
    <link rel="assets" href="https://a248.e.akamai.net/assets.github.com/">
    <link rel="xhr-socket" href="/_sockets" />
    


    <meta name="msapplication-TileImage" content="/windows-tile.png" />
    <meta name="msapplication-TileColor" content="#ffffff" />
    <meta name="selected-link" value="repo_source" data-pjax-transient />
    <meta content="collector.githubapp.com" name="octolytics-host" /><meta content="github" name="octolytics-app-id" />

    
    
    <link rel="icon" type="image/x-icon" href="/favicon.ico" />

    <meta content="authenticity_token" name="csrf-param" />
<meta content="AsRqPU4iFZZjq8AJIBt9HwSBTyplOdWrAYBVk1Lmg5A=" name="csrf-token" />

    <link href="https://a248.e.akamai.net/assets.github.com/assets/github-48bea3124ce323ebd49dba9db4f54434cfb49307.css" media="all" rel="stylesheet" type="text/css" />
    <link href="https://a248.e.akamai.net/assets.github.com/assets/github2-fc192c2a2984e35a3501881e687042978a243b20.css" media="all" rel="stylesheet" type="text/css" />
    


      <script src="https://a248.e.akamai.net/assets.github.com/assets/frameworks-1f72571b966545f4e27481a3b0ebbeeed4f2f139.js" type="text/javascript"></script>
      <script src="https://a248.e.akamai.net/assets.github.com/assets/github-478f5296ed432222e86933b8579220df11d72a0f.js" type="text/javascript"></script>
      
      <meta http-equiv="x-pjax-version" content="25e506f9d180fad9a0722371c764d021">

        <link data-pjax-transient rel='permalink' href='/numb3r23/glslAdditions/blob/5ede089b5fe8a5de0efe52d12ff03f58fb9e070e/doxygen/glslfilter.py'>

  <meta property="og:title" content="glslAdditions"/>
  <meta property="og:type" content="githubog:gitrepository"/>
  <meta property="og:url" content="https://github.com/numb3r23/glslAdditions"/>
  <meta property="og:image" content="https://a248.e.akamai.net/assets.github.com/images/gravatars/gravatar-user-420.png"/>
  <meta property="og:site_name" content="GitHub"/>
  <meta property="og:description" content="glslAdditions - Various useful stuff for working with GLSL shaders"/>

  <meta name="description" content="glslAdditions - Various useful stuff for working with GLSL shaders" />

  <meta content="658343" name="octolytics-dimension-user_id" /><meta content="numb3r23" name="octolytics-dimension-user_login" /><meta content="7450020" name="octolytics-dimension-repository_id" /><meta content="numb3r23/glslAdditions" name="octolytics-dimension-repository_nwo" /><meta content="true" name="octolytics-dimension-repository_public" /><meta content="false" name="octolytics-dimension-repository_is_fork" /><meta content="7450020" name="octolytics-dimension-repository_network_root_id" /><meta content="numb3r23/glslAdditions" name="octolytics-dimension-repository_network_root_nwo" />
  <link href="https://github.com/numb3r23/glslAdditions/commits/master.atom" rel="alternate" title="Recent Commits to glslAdditions:master" type="application/atom+xml" />

  </head>


  <body class="logged_out page-blob windows vis-public env-production  kill-the-chrome">

    <div class="wrapper">
      
      
      

      
      <div class="header header-logged-out">
  <div class="container clearfix">

    <a class="header-logo-wordmark" href="https://github.com/">
      <span class="mega-octicon octicon-logo-github"></span>
    </a>

    <div class="header-actions">
      <a class="button primary" href="/signup">Sign up</a>
      <a class="button" href="/login?return_to=%2Fnumb3r23%2FglslAdditions%2Fblob%2Fmaster%2Fdoxygen%2Fglslfilter.py">Sign in</a>
    </div>

    <div class="command-bar js-command-bar  in-repository">


      <ul class="top-nav">
          <li class="explore"><a href="/explore">Explore</a></li>
        <li class="features"><a href="/features">Features</a></li>
          <li class="enterprise"><a href="https://enterprise.github.com/">Enterprise</a></li>
          <li class="blog"><a href="/blog">Blog</a></li>
      </ul>
        <form accept-charset="UTF-8" action="/search" class="command-bar-form" id="top_search_form" method="get">

<input type="text" data-hotkey="/ s" name="q" id="js-command-bar-field" placeholder="Search or type a command" tabindex="1" autocapitalize="off"
    
    
      data-repo="numb3r23/glslAdditions"
      data-branch="master"
      data-sha="f8701a2a45c55d5222a0e2c4d8eea30f85e90fa6"
  >

    <input type="hidden" name="nwo" value="numb3r23/glslAdditions" />

    <div class="select-menu js-menu-container js-select-menu search-context-select-menu">
      <span class="minibutton select-menu-button js-menu-target">
        <span class="js-select-button">This repository</span>
      </span>

      <div class="select-menu-modal-holder js-menu-content js-navigation-container">
        <div class="select-menu-modal">

          <div class="select-menu-item js-navigation-item selected">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" class="js-search-this-repository" name="search_target" value="repository" checked="checked" />
            <div class="select-menu-item-text js-select-button-text">This repository</div>
          </div> <!-- /.select-menu-item -->

          <div class="select-menu-item js-navigation-item">
            <span class="select-menu-item-icon octicon octicon-check"></span>
            <input type="radio" name="search_target" value="global" />
            <div class="select-menu-item-text js-select-button-text">All repositories</div>
          </div> <!-- /.select-menu-item -->

        </div>
      </div>
    </div>

  <span class="octicon help tooltipped downwards" title="Show command bar help">
    <span class="octicon octicon-question"></span>
  </span>


  <input type="hidden" name="ref" value="cmdform">

</form>
    </div>

  </div>
</div>


      


          <div class="site" itemscope itemtype="http://schema.org/WebPage">
    
    <div class="pagehead repohead instapaper_ignore readability-menu">
      <div class="container">
        

<ul class="pagehead-actions">



    <li>
      <a href="/login?return_to=%2Fnumb3r23%2FglslAdditions"
        class="minibutton with-count js-toggler-target star-button entice tooltipped upwards"
        title="You must be signed in to use this feature" rel="nofollow">
        <span class="octicon octicon-star"></span>Star
      </a>
      <a class="social-count js-social-count" href="/numb3r23/glslAdditions/stargazers">
        2
      </a>
    </li>
    <li>
      <a href="/login?return_to=%2Fnumb3r23%2FglslAdditions"
        class="minibutton with-count js-toggler-target fork-button entice tooltipped upwards"
        title="You must be signed in to fork a repository" rel="nofollow">
        <span class="octicon octicon-git-branch"></span>Fork
      </a>
      <a href="/numb3r23/glslAdditions/network" class="social-count">
        1
      </a>
    </li>
</ul>

        <h1 itemscope itemtype="http://data-vocabulary.org/Breadcrumb" class="entry-title public">
          <span class="repo-label"><span>public</span></span>
          <span class="mega-octicon octicon-repo"></span>
          <span class="author">
            <a href="/numb3r23" class="url fn" itemprop="url" rel="author"><span itemprop="title">numb3r23</span></a></span
          ><span class="repohead-name-divider">/</span><strong
          ><a href="/numb3r23/glslAdditions" class="js-current-repository js-repo-home-link">glslAdditions</a></strong>

          <span class="page-context-loader">
            <img alt="Octocat-spinner-32" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
          </span>

        </h1>
      </div><!-- /.container -->
    </div><!-- /.repohead -->

    <div class="container">

      <div class="repository-with-sidebar repo-container
            ">

          <div class="repository-sidebar">

              

<div class="repo-nav repo-nav-full js-repository-container-pjax js-octicon-loaders">
  <div class="repo-nav-contents">
    <ul class="repo-menu">
      <li class="tooltipped leftwards" title="Code">
        <a href="/numb3r23/glslAdditions" class="js-selected-navigation-item selected" data-gotokey="c" data-pjax="true" data-selected-links="repo_source repo_downloads repo_commits repo_tags repo_branches /numb3r23/glslAdditions">
          <span class="octicon octicon-code"></span> <span class="full-word">Code</span>
          <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

        <li class="tooltipped leftwards" title="Issues">
          <a href="/numb3r23/glslAdditions/issues" class="js-selected-navigation-item js-disable-pjax" data-gotokey="i" data-selected-links="repo_issues /numb3r23/glslAdditions/issues">
            <span class="octicon octicon-issue-opened"></span> <span class="full-word">Issues</span>
            <span class='counter'>0</span>
            <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>        </li>

      <li class="tooltipped leftwards" title="Pull Requests"><a href="/numb3r23/glslAdditions/pulls" class="js-selected-navigation-item js-disable-pjax" data-gotokey="p" data-selected-links="repo_pulls /numb3r23/glslAdditions/pulls">
            <span class="octicon octicon-git-pull-request"></span> <span class="full-word">Pull Requests</span>
            <span class='counter'>0</span>
            <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>




    </ul>
    <div class="repo-menu-separator"></div>
    <ul class="repo-menu">

      <li class="tooltipped leftwards" title="Pulse">
        <a href="/numb3r23/glslAdditions/pulse" class="js-selected-navigation-item " data-pjax="true" data-selected-links="pulse /numb3r23/glslAdditions/pulse">
          <span class="octicon octicon-pulse"></span> <span class="full-word">Pulse</span>
          <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped leftwards" title="Graphs">
        <a href="/numb3r23/glslAdditions/graphs" class="js-selected-navigation-item " data-pjax="true" data-selected-links="repo_graphs repo_contributors /numb3r23/glslAdditions/graphs">
          <span class="octicon octicon-graph"></span> <span class="full-word">Graphs</span>
          <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped leftwards" title="Network">
        <a href="/numb3r23/glslAdditions/network" class="js-selected-navigation-item js-disable-pjax" data-selected-links="repo_network /numb3r23/glslAdditions/network">
          <span class="octicon octicon-git-branch"></span> <span class="full-word">Network</span>
          <img alt="Octocat-spinner-32" class="mini-loader" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

    </ul>

  </div>
</div>


              <div class="only-with-full-nav">

                

  

<div class="clone-url open"
  data-protocol-type="http"
  data-url="/users/set_protocol?protocol_selector=http&amp;protocol_type=clone">
  <h3><strong>HTTPS</strong> clone URL</h3>

  <input type="text" class="clone js-url-field"
         value="https://github.com/numb3r23/glslAdditions.git" readonly="readonly">

  <span class="js-zeroclipboard url-box-clippy zeroclipboard-button" data-clipboard-text="https://github.com/numb3r23/glslAdditions.git" data-copied-hint="copied!" title="copy to clipboard"><span class="octicon octicon-clippy"></span></span>
</div>

  

<div class="clone-url "
  data-protocol-type="subversion"
  data-url="/users/set_protocol?protocol_selector=subversion&amp;protocol_type=clone">
  <h3><strong>Subversion</strong> checkout URL</h3>

  <input type="text" class="clone js-url-field"
         value="https://github.com/numb3r23/glslAdditions" readonly="readonly">

  <span class="js-zeroclipboard url-box-clippy zeroclipboard-button" data-clipboard-text="https://github.com/numb3r23/glslAdditions" data-copied-hint="copied!" title="copy to clipboard"><span class="octicon octicon-clippy"></span></span>
</div>



<p class="clone-options">You can clone with
    <a href="#" class="js-clone-selector" data-protocol="http">HTTPS</a>,
    <a href="#" class="js-clone-selector" data-protocol="subversion">Subversion</a>,
  and <a href="https://help.github.com/articles/which-remote-url-should-i-use">other methods.</a>
</p>


  <a href="http://windows.github.com" class="minibutton sidebar-button">
    <span class="octicon octicon-device-desktop"></span>
    Clone in Desktop
  </a>


                  <a href="/numb3r23/glslAdditions/archive/master.zip"
                     class="minibutton sidebar-button"
                     title="Download this repository as a zip file"
                     rel="nofollow">
                    <span class="octicon octicon-cloud-download"></span>
                    Download ZIP
                  </a>

              </div>
          </div>

          <div id="js-repo-pjax-container" class="repository-content context-loader-container" data-pjax-container>
            


<!-- blob contrib key: blob_contributors:v21:15f9ef13132a0fee51b19a92c3f50163 -->
<!-- blob contrib frag key: views10/v8/blob_contributors:v21:15f9ef13132a0fee51b19a92c3f50163 -->


      <p title="This is a placeholder element" class="js-history-link-replace hidden"></p>

        <a href="/numb3r23/glslAdditions/find/master" data-pjax data-hotkey="t" style="display:none">Show File Finder</a>

        <div class="file-navigation">
          


<div class="select-menu js-menu-container js-select-menu" >
  <span class="minibutton select-menu-button js-menu-target" data-hotkey="w"
    data-master-branch="master"
    data-ref="master">
    <span class="octicon octicon-git-branch"></span>
    <i>branch:</i>
    <span class="js-select-button">master</span>
  </span>

  <div class="select-menu-modal-holder js-menu-content js-navigation-container" data-pjax>

    <div class="select-menu-modal">
      <div class="select-menu-header">
        <span class="select-menu-title">Switch branches/tags</span>
        <span class="octicon octicon-remove-close js-menu-close"></span>
      </div> <!-- /.select-menu-header -->

      <div class="select-menu-filters">
        <div class="select-menu-text-filter">
          <input type="text" id="context-commitish-filter-field" class="js-filterable-field js-navigation-enable" placeholder="Filter branches/tags">
        </div>
        <div class="select-menu-tabs">
          <ul>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="branches" class="js-select-menu-tab">Branches</a>
            </li>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="tags" class="js-select-menu-tab">Tags</a>
            </li>
          </ul>
        </div><!-- /.select-menu-tabs -->
      </div><!-- /.select-menu-filters -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="branches">

        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <div class="select-menu-item js-navigation-item selected">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/numb3r23/glslAdditions/blob/master/doxygen/glslfilter.py" class="js-navigation-open select-menu-item-text js-select-button-text css-truncate-target" data-name="master" rel="nofollow" title="master">master</a>
            </div> <!-- /.select-menu-item -->
        </div>

          <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="tags">
        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


        </div>

        <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

    </div> <!-- /.select-menu-modal -->
  </div> <!-- /.select-menu-modal-holder -->
</div> <!-- /.select-menu -->

          <div class="breadcrumb">
            <span class='repo-root js-repo-root'><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/numb3r23/glslAdditions" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">glslAdditions</span></a></span></span><span class="separator"> / </span><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/numb3r23/glslAdditions/tree/master/doxygen" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">doxygen</span></a></span><span class="separator"> / </span><strong class="final-path">glslfilter.py</strong> <span class="js-zeroclipboard zeroclipboard-button" data-clipboard-text="doxygen/glslfilter.py" data-copied-hint="copied!" title="copy to clipboard"><span class="octicon octicon-clippy"></span></span>
          </div>
        </div>


        <div class="commit commit-loader file-history-tease js-deferred-content" data-url="/numb3r23/glslAdditions/contributors/master/doxygen/glslfilter.py">
          Fetching contributors…

          <div class="participation">
            <p class="loader-loading"><img alt="Octocat-spinner-32-eaf2f5" height="16" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-32-EAF2F5.gif" width="16" /></p>
            <p class="loader-error">Cannot retrieve contributors at this time</p>
          </div>
        </div>


        <div id="files" class="bubble">
          <div class="file">
            <div class="meta">
              <div class="info">
                <span class="icon"><b class="octicon octicon-file-text"></b></span>
                <span class="mode" title="File Mode">file</span>
                  <span>159 lines (138 sloc)</span>
                <span>4.938 kb</span>
              </div>
              <div class="actions">
                <div class="button-group">
                      <a class="minibutton js-entice" href=""
                         data-entice="You must be signed in and on a branch to make or propose changes">Edit</a>
                  <a href="/numb3r23/glslAdditions/raw/master/doxygen/glslfilter.py" class="button minibutton " id="raw-url">Raw</a>
                    <a href="/numb3r23/glslAdditions/blame/master/doxygen/glslfilter.py" class="button minibutton ">Blame</a>
                  <a href="/numb3r23/glslAdditions/commits/master/doxygen/glslfilter.py" class="button minibutton " rel="nofollow">History</a>
                </div><!-- /.button-group -->
              </div><!-- /.actions -->

            </div>
                <div class="blob-wrapper data type-python js-blob-data">
      <table class="file-code file-diff">
        <tr class="file-code-line">
          <td class="blob-line-nums">
            <span id="L1" rel="#L1">1</span>
<span id="L2" rel="#L2">2</span>
<span id="L3" rel="#L3">3</span>
<span id="L4" rel="#L4">4</span>
<span id="L5" rel="#L5">5</span>
<span id="L6" rel="#L6">6</span>
<span id="L7" rel="#L7">7</span>
<span id="L8" rel="#L8">8</span>
<span id="L9" rel="#L9">9</span>
<span id="L10" rel="#L10">10</span>
<span id="L11" rel="#L11">11</span>
<span id="L12" rel="#L12">12</span>
<span id="L13" rel="#L13">13</span>
<span id="L14" rel="#L14">14</span>
<span id="L15" rel="#L15">15</span>
<span id="L16" rel="#L16">16</span>
<span id="L17" rel="#L17">17</span>
<span id="L18" rel="#L18">18</span>
<span id="L19" rel="#L19">19</span>
<span id="L20" rel="#L20">20</span>
<span id="L21" rel="#L21">21</span>
<span id="L22" rel="#L22">22</span>
<span id="L23" rel="#L23">23</span>
<span id="L24" rel="#L24">24</span>
<span id="L25" rel="#L25">25</span>
<span id="L26" rel="#L26">26</span>
<span id="L27" rel="#L27">27</span>
<span id="L28" rel="#L28">28</span>
<span id="L29" rel="#L29">29</span>
<span id="L30" rel="#L30">30</span>
<span id="L31" rel="#L31">31</span>
<span id="L32" rel="#L32">32</span>
<span id="L33" rel="#L33">33</span>
<span id="L34" rel="#L34">34</span>
<span id="L35" rel="#L35">35</span>
<span id="L36" rel="#L36">36</span>
<span id="L37" rel="#L37">37</span>
<span id="L38" rel="#L38">38</span>
<span id="L39" rel="#L39">39</span>
<span id="L40" rel="#L40">40</span>
<span id="L41" rel="#L41">41</span>
<span id="L42" rel="#L42">42</span>
<span id="L43" rel="#L43">43</span>
<span id="L44" rel="#L44">44</span>
<span id="L45" rel="#L45">45</span>
<span id="L46" rel="#L46">46</span>
<span id="L47" rel="#L47">47</span>
<span id="L48" rel="#L48">48</span>
<span id="L49" rel="#L49">49</span>
<span id="L50" rel="#L50">50</span>
<span id="L51" rel="#L51">51</span>
<span id="L52" rel="#L52">52</span>
<span id="L53" rel="#L53">53</span>
<span id="L54" rel="#L54">54</span>
<span id="L55" rel="#L55">55</span>
<span id="L56" rel="#L56">56</span>
<span id="L57" rel="#L57">57</span>
<span id="L58" rel="#L58">58</span>
<span id="L59" rel="#L59">59</span>
<span id="L60" rel="#L60">60</span>
<span id="L61" rel="#L61">61</span>
<span id="L62" rel="#L62">62</span>
<span id="L63" rel="#L63">63</span>
<span id="L64" rel="#L64">64</span>
<span id="L65" rel="#L65">65</span>
<span id="L66" rel="#L66">66</span>
<span id="L67" rel="#L67">67</span>
<span id="L68" rel="#L68">68</span>
<span id="L69" rel="#L69">69</span>
<span id="L70" rel="#L70">70</span>
<span id="L71" rel="#L71">71</span>
<span id="L72" rel="#L72">72</span>
<span id="L73" rel="#L73">73</span>
<span id="L74" rel="#L74">74</span>
<span id="L75" rel="#L75">75</span>
<span id="L76" rel="#L76">76</span>
<span id="L77" rel="#L77">77</span>
<span id="L78" rel="#L78">78</span>
<span id="L79" rel="#L79">79</span>
<span id="L80" rel="#L80">80</span>
<span id="L81" rel="#L81">81</span>
<span id="L82" rel="#L82">82</span>
<span id="L83" rel="#L83">83</span>
<span id="L84" rel="#L84">84</span>
<span id="L85" rel="#L85">85</span>
<span id="L86" rel="#L86">86</span>
<span id="L87" rel="#L87">87</span>
<span id="L88" rel="#L88">88</span>
<span id="L89" rel="#L89">89</span>
<span id="L90" rel="#L90">90</span>
<span id="L91" rel="#L91">91</span>
<span id="L92" rel="#L92">92</span>
<span id="L93" rel="#L93">93</span>
<span id="L94" rel="#L94">94</span>
<span id="L95" rel="#L95">95</span>
<span id="L96" rel="#L96">96</span>
<span id="L97" rel="#L97">97</span>
<span id="L98" rel="#L98">98</span>
<span id="L99" rel="#L99">99</span>
<span id="L100" rel="#L100">100</span>
<span id="L101" rel="#L101">101</span>
<span id="L102" rel="#L102">102</span>
<span id="L103" rel="#L103">103</span>
<span id="L104" rel="#L104">104</span>
<span id="L105" rel="#L105">105</span>
<span id="L106" rel="#L106">106</span>
<span id="L107" rel="#L107">107</span>
<span id="L108" rel="#L108">108</span>
<span id="L109" rel="#L109">109</span>
<span id="L110" rel="#L110">110</span>
<span id="L111" rel="#L111">111</span>
<span id="L112" rel="#L112">112</span>
<span id="L113" rel="#L113">113</span>
<span id="L114" rel="#L114">114</span>
<span id="L115" rel="#L115">115</span>
<span id="L116" rel="#L116">116</span>
<span id="L117" rel="#L117">117</span>
<span id="L118" rel="#L118">118</span>
<span id="L119" rel="#L119">119</span>
<span id="L120" rel="#L120">120</span>
<span id="L121" rel="#L121">121</span>
<span id="L122" rel="#L122">122</span>
<span id="L123" rel="#L123">123</span>
<span id="L124" rel="#L124">124</span>
<span id="L125" rel="#L125">125</span>
<span id="L126" rel="#L126">126</span>
<span id="L127" rel="#L127">127</span>
<span id="L128" rel="#L128">128</span>
<span id="L129" rel="#L129">129</span>
<span id="L130" rel="#L130">130</span>
<span id="L131" rel="#L131">131</span>
<span id="L132" rel="#L132">132</span>
<span id="L133" rel="#L133">133</span>
<span id="L134" rel="#L134">134</span>
<span id="L135" rel="#L135">135</span>
<span id="L136" rel="#L136">136</span>
<span id="L137" rel="#L137">137</span>
<span id="L138" rel="#L138">138</span>
<span id="L139" rel="#L139">139</span>
<span id="L140" rel="#L140">140</span>
<span id="L141" rel="#L141">141</span>
<span id="L142" rel="#L142">142</span>
<span id="L143" rel="#L143">143</span>
<span id="L144" rel="#L144">144</span>
<span id="L145" rel="#L145">145</span>
<span id="L146" rel="#L146">146</span>
<span id="L147" rel="#L147">147</span>
<span id="L148" rel="#L148">148</span>
<span id="L149" rel="#L149">149</span>
<span id="L150" rel="#L150">150</span>
<span id="L151" rel="#L151">151</span>
<span id="L152" rel="#L152">152</span>
<span id="L153" rel="#L153">153</span>
<span id="L154" rel="#L154">154</span>
<span id="L155" rel="#L155">155</span>
<span id="L156" rel="#L156">156</span>
<span id="L157" rel="#L157">157</span>
<span id="L158" rel="#L158">158</span>

          </td>
          <td class="blob-line-code">
                  <div class="highlight"><pre><div class='line' id='LC1'><span class="c">#!/usr/bin/env python</span></div><div class='line' id='LC2'><span class="c"># -*- coding: utf-8 -*- </span></div><div class='line' id='LC3'><span class="c">#</span></div><div class='line' id='LC4'><br/></div><div class='line' id='LC5'><span class="c"># Copyright notice: </span></div><div class='line' id='LC6'><span class="c"># This program is free software; you can redistribute it and/or</span></div><div class='line' id='LC7'><span class="c"># modify it under the terms of the GNU General Public License</span></div><div class='line' id='LC8'><span class="c"># as published by the Free Software Foundation; either version 2</span></div><div class='line' id='LC9'><span class="c"># of the License, or (at your option) any later version.</span></div><div class='line' id='LC10'><span class="c"># </span></div><div class='line' id='LC11'><span class="c"># This program is distributed in the hope that it will be useful,</span></div><div class='line' id='LC12'><span class="c"># but WITHOUT ANY WARRANTY; without even the implied warranty of</span></div><div class='line' id='LC13'><span class="c"># MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the</span></div><div class='line' id='LC14'><span class="c"># GNU General Public License for more details.</span></div><div class='line' id='LC15'><span class="c"># </span></div><div class='line' id='LC16'><span class="c"># You should have received a copy of the GNU General Public License</span></div><div class='line' id='LC17'><span class="c"># along with this program; if not, write to the Free Software</span></div><div class='line' id='LC18'><span class="c"># Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.</span></div><div class='line' id='LC19'><br/></div><div class='line' id='LC20'><span class="kn">import</span> <span class="nn">getopt</span>          <span class="c"># get command-line options</span></div><div class='line' id='LC21'><span class="kn">import</span> <span class="nn">os.path</span>         <span class="c"># getting extension from file</span></div><div class='line' id='LC22'><span class="kn">import</span> <span class="nn">string</span>          <span class="c"># string manipulation</span></div><div class='line' id='LC23'><span class="kn">import</span> <span class="nn">sys</span>             <span class="c"># output and stuff</span></div><div class='line' id='LC24'><span class="kn">import</span> <span class="nn">re</span>              <span class="c"># for regular expressions</span></div><div class='line' id='LC25'><br/></div><div class='line' id='LC26'><br/></div><div class='line' id='LC27'><span class="c">## extract doxygen-tag class</span></div><div class='line' id='LC28'><span class="n">re_doxy_class</span> <span class="o">=</span> <span class="n">re</span><span class="o">.</span><span class="n">compile</span><span class="p">(</span><span class="s">&#39;(?&lt;=[@]class\s)\w+&#39;</span><span class="p">,</span> <span class="n">re</span><span class="o">.</span><span class="n">I</span> <span class="o">|</span> <span class="n">re</span><span class="o">.</span><span class="n">VERBOSE</span><span class="p">)</span></div><div class='line' id='LC29'><span class="c">## extract doxygen-tag namespace</span></div><div class='line' id='LC30'><span class="n">re_doxy_namespace</span> <span class="o">=</span> <span class="n">re</span><span class="o">.</span><span class="n">compile</span><span class="p">(</span><span class="s">&#39;(?&lt;=[@]namespace\s)[^\s]+&#39;</span><span class="p">,</span> <span class="n">re</span><span class="o">.</span><span class="n">I</span> <span class="o">|</span> <span class="n">re</span><span class="o">.</span><span class="n">VERBOSE</span><span class="p">)</span></div><div class='line' id='LC31'><span class="n">re_blockcode_start</span> <span class="o">=</span> <span class="n">re</span><span class="o">.</span><span class="n">compile</span><span class="p">(</span><span class="s">&#39;(?&lt;=[\*]class\s)\w+&#39;</span><span class="p">,</span> <span class="n">re</span><span class="o">.</span><span class="n">I</span> <span class="o">|</span> <span class="n">re</span><span class="o">.</span><span class="n">VERBOSE</span><span class="p">)</span></div><div class='line' id='LC32'><br/></div><div class='line' id='LC33'><span class="c">##</span></div><div class='line' id='LC34'><span class="c"># @package glslfilter</span></div><div class='line' id='LC35'><span class="c"># @brief A Doxygen filter to document GLSL-Shader, based on a vb-filter from Basti Grembowietz</span></div><div class='line' id='LC36'><span class="c"># @author Sebastian Schäfer</span></div><div class='line' id='LC37'><span class="c"># @date 02/2012</span></div><div class='line' id='LC38'><span class="c"># @version 0.1</span></div><div class='line' id='LC39'><span class="c"># @copyright GNU Public License.</span></div><div class='line' id='LC40'><span class="c"># </span></div><div class='line' id='LC41'><span class="c"># @details The shader file is wrapped into a class and namespace that can be set with </span></div><div class='line' id='LC42'><span class="c"># doxygen-tags.</span></div><div class='line' id='LC43'><span class="c"># </span></div><div class='line' id='LC44'><span class="c"># Usage:</span></div><div class='line' id='LC45'><span class="c"># - shader file:</span></div><div class='line' id='LC46'><span class="c">#   - set doxygen name for class name -&gt; defaults to filename</span></div><div class='line' id='LC47'><span class="c">#   - set doxygen namespace for namespace (pseudo category) -&gt; defaults to GLSL</span></div><div class='line' id='LC48'><span class="c"># - doxygen file:</span></div><div class='line' id='LC49'><span class="c">#   - add FILE_PATTERNS: *.frag, *.vert</span></div><div class='line' id='LC50'><span class="c">#   - add FILTER_PATTERNS: &quot;*.frag=glslfilter.py&quot;, &quot;*.vert=glslfilter.py&quot;</span></div><div class='line' id='LC51'><span class="c"># latest version on &lt;a href=&quot;http://www.numb3r23.net&quot;&gt;www.numb3r23.net&lt;/a&gt;</span></div><div class='line' id='LC52'><br/></div><div class='line' id='LC53'><span class="c">##run regex on a single line</span></div><div class='line' id='LC54'><span class="c"># @returns either a found result or None</span></div><div class='line' id='LC55'><span class="k">def</span> <span class="nf">getRegSearchLine</span><span class="p">(</span><span class="nb">str</span><span class="p">,</span> <span class="n">regex</span><span class="p">):</span></div><div class='line' id='LC56'>	<span class="n">search</span> <span class="o">=</span> <span class="n">regex</span><span class="o">.</span><span class="n">search</span><span class="p">(</span><span class="nb">str</span><span class="p">)</span></div><div class='line' id='LC57'>	<span class="k">if</span> <span class="n">search</span> <span class="ow">is</span> <span class="ow">not</span> <span class="bp">None</span><span class="p">:</span></div><div class='line' id='LC58'>		<span class="k">return</span> <span class="n">search</span><span class="o">.</span><span class="n">group</span><span class="p">(</span><span class="mi">0</span><span class="p">)</span></div><div class='line' id='LC59'>	<span class="k">return</span> <span class="bp">None</span></div><div class='line' id='LC60'><br/></div><div class='line' id='LC61'><span class="c">##run regex on an string array</span></div><div class='line' id='LC62'><span class="c"># @returns either a found result or None</span></div><div class='line' id='LC63'><span class="k">def</span> <span class="nf">getRegSearch</span><span class="p">(</span><span class="n">txt</span><span class="p">,</span> <span class="n">regex</span><span class="p">):</span></div><div class='line' id='LC64'>	<span class="k">for</span> <span class="nb">str</span> <span class="ow">in</span> <span class="n">txt</span><span class="p">:</span></div><div class='line' id='LC65'>		<span class="n">search</span> <span class="o">=</span> <span class="n">regex</span><span class="o">.</span><span class="n">search</span><span class="p">(</span><span class="nb">str</span><span class="p">)</span></div><div class='line' id='LC66'>		<span class="k">if</span> <span class="n">search</span> <span class="ow">is</span> <span class="ow">not</span> <span class="bp">None</span><span class="p">:</span></div><div class='line' id='LC67'>			<span class="k">return</span> <span class="n">search</span><span class="o">.</span><span class="n">group</span><span class="p">(</span><span class="mi">0</span><span class="p">)</span></div><div class='line' id='LC68'>	<span class="k">return</span> <span class="bp">None</span></div><div class='line' id='LC69'><br/></div><div class='line' id='LC70'><span class="c"># generate a class name from filename</span></div><div class='line' id='LC71'><span class="c"># @return just the filename - no extension and no path</span></div><div class='line' id='LC72'><span class="k">def</span> <span class="nf">generateName</span><span class="p">(</span><span class="n">filename</span><span class="p">):</span></div><div class='line' id='LC73'>	<span class="n">root</span><span class="p">,</span> <span class="n">ext</span> <span class="o">=</span> <span class="n">os</span><span class="o">.</span><span class="n">path</span><span class="o">.</span><span class="n">splitext</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC74'>	<span class="n">head</span><span class="p">,</span> <span class="n">tail</span> <span class="o">=</span> <span class="n">os</span><span class="o">.</span><span class="n">path</span><span class="o">.</span><span class="n">split</span><span class="p">(</span><span class="n">root</span><span class="p">)</span></div><div class='line' id='LC75'>	<span class="k">return</span> <span class="n">tail</span></div><div class='line' id='LC76'><br/></div><div class='line' id='LC77'><span class="k">def</span> <span class="nf">writeLine</span><span class="p">(</span><span class="n">txt</span><span class="p">):</span></div><div class='line' id='LC78'>	<span class="n">sys</span><span class="o">.</span><span class="n">stdout</span><span class="o">.</span><span class="n">write</span><span class="p">(</span><span class="n">txt</span> <span class="o">+</span> <span class="s">&quot;</span><span class="se">\n</span><span class="s">&quot;</span><span class="p">)</span></div><div class='line' id='LC79'><br/></div><div class='line' id='LC80'><span class="c">## parse a shader and generate needed information along on the way</span></div><div class='line' id='LC81'><span class="c">## - if comments contain a namespace move it the classname</span></div><div class='line' id='LC82'><span class="k">def</span> <span class="nf">parseShader</span><span class="p">(</span><span class="n">filename</span><span class="p">,</span> <span class="n">txt</span><span class="p">,</span> <span class="nb">type</span> <span class="o">=</span> <span class="bp">None</span><span class="p">):</span></div><div class='line' id='LC83'>	<span class="c"># extract name from doxygen-tag or use generic GLSL namespace</span></div><div class='line' id='LC84'>	<span class="n">namespace</span> <span class="o">=</span> <span class="n">getRegSearch</span><span class="p">(</span><span class="n">txt</span><span class="p">,</span> <span class="n">re_doxy_namespace</span><span class="p">)</span></div><div class='line' id='LC85'>	<span class="k">if</span> <span class="n">namespace</span> <span class="ow">is</span> <span class="bp">None</span><span class="p">:</span></div><div class='line' id='LC86'>		<span class="n">namespace</span> <span class="o">=</span> <span class="s">&quot;GLSL&quot;</span></div><div class='line' id='LC87'>	<span class="k">else</span><span class="p">:</span></div><div class='line' id='LC88'>		<span class="c">#remove namespace line from txt</span></div><div class='line' id='LC89'>		<span class="n">txt</span> <span class="o">=</span> <span class="p">[</span><span class="nb">str</span> <span class="k">for</span> <span class="nb">str</span> <span class="ow">in</span> <span class="n">txt</span> <span class="k">if</span> <span class="n">getRegSearchLine</span><span class="p">(</span><span class="nb">str</span><span class="p">,</span> <span class="n">re_doxy_namespace</span><span class="p">)</span> <span class="ow">is</span> <span class="bp">None</span><span class="p">]</span></div><div class='line' id='LC90'><br/></div><div class='line' id='LC91'>	<span class="c"># extract className from doxygen-tag or use filename</span></div><div class='line' id='LC92'>	<span class="n">className</span> <span class="o">=</span> <span class="n">getRegSearch</span><span class="p">(</span><span class="n">txt</span><span class="p">,</span> <span class="n">re_doxy_class</span><span class="p">)</span></div><div class='line' id='LC93'>	<span class="k">if</span> <span class="n">className</span> <span class="ow">is</span> <span class="bp">None</span><span class="p">:</span></div><div class='line' id='LC94'>		<span class="n">className</span> <span class="o">=</span> <span class="n">generateName</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC95'>	<span class="k">else</span><span class="p">:</span></div><div class='line' id='LC96'>		<span class="c">#remove calssName line from txt</span></div><div class='line' id='LC97'>		<span class="n">txt</span> <span class="o">=</span> <span class="p">[</span><span class="nb">str</span> <span class="k">for</span> <span class="nb">str</span> <span class="ow">in</span> <span class="n">txt</span> <span class="k">if</span> <span class="n">getRegSearchLine</span><span class="p">(</span><span class="nb">str</span><span class="p">,</span> <span class="n">re_doxy_class</span><span class="p">)</span> <span class="ow">is</span> <span class="bp">None</span><span class="p">]</span></div><div class='line' id='LC98'><br/></div><div class='line' id='LC99'>	<span class="n">comment</span> <span class="o">=</span> <span class="p">[]</span></div><div class='line' id='LC100'>	<span class="k">if</span> <span class="nb">len</span><span class="p">(</span><span class="n">txt</span><span class="p">)</span> <span class="o">&gt;</span> <span class="mi">0</span><span class="p">:</span></div><div class='line' id='LC101'>		<span class="k">if</span> <span class="n">txt</span><span class="p">[</span><span class="mi">0</span><span class="p">]</span><span class="o">.</span><span class="n">find</span><span class="p">(</span><span class="s">&quot;/*&quot;</span><span class="p">)</span> <span class="o">&gt;=</span><span class="mi">0</span><span class="p">:</span></div><div class='line' id='LC102'>			<span class="n">line</span> <span class="o">=</span> <span class="n">txt</span><span class="o">.</span><span class="n">pop</span><span class="p">(</span><span class="mi">0</span><span class="p">)</span></div><div class='line' id='LC103'>			<span class="k">while</span> <span class="p">(</span><span class="n">line</span><span class="o">.</span><span class="n">find</span><span class="p">(</span><span class="s">&quot;*/&quot;</span><span class="p">)</span> <span class="o">&lt;</span> <span class="mi">0</span><span class="p">)</span> <span class="ow">and</span> <span class="p">(</span><span class="nb">len</span><span class="p">(</span><span class="n">txt</span><span class="p">)</span> <span class="o">&gt;</span> <span class="mi">0</span><span class="p">):</span></div><div class='line' id='LC104'>				<span class="n">comment</span><span class="o">.</span><span class="n">append</span><span class="p">(</span><span class="n">line</span><span class="p">)</span></div><div class='line' id='LC105'>				<span class="n">line</span> <span class="o">=</span> <span class="n">txt</span><span class="o">.</span><span class="n">pop</span><span class="p">(</span><span class="mi">0</span><span class="p">)</span></div><div class='line' id='LC106'>			<span class="n">comment</span><span class="o">.</span><span class="n">append</span><span class="p">(</span><span class="n">line</span><span class="p">)</span></div><div class='line' id='LC107'><br/></div><div class='line' id='LC108'><br/></div><div class='line' id='LC109'><br/></div><div class='line' id='LC110'>	<span class="c"># dump the file and pad it with namespace/name class information</span></div><div class='line' id='LC111'>	<span class="c"># 1st: namespace + class padding, also declare everything public</span></div><div class='line' id='LC112'>	<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;/** @namespace &quot;</span> <span class="o">+</span> <span class="n">namespace</span> <span class="o">+</span> <span class="s">&quot; */&quot;</span><span class="p">)</span></div><div class='line' id='LC113'>	<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;public class &quot;</span> <span class="o">+</span> <span class="n">namespace</span><span class="o">+</span><span class="s">&quot;::&quot;</span><span class="o">+</span><span class="n">className</span> <span class="o">+</span> <span class="s">&quot;{&quot;</span><span class="p">)</span></div><div class='line' id='LC114'>	<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;public:&quot;</span><span class="p">)</span></div><div class='line' id='LC115'>	<span class="c"># 2nd: dump original commentblock</span></div><div class='line' id='LC116'>	<span class="n">showLines</span><span class="p">(</span><span class="n">comment</span><span class="p">)</span></div><div class='line' id='LC117'>	<span class="c"># 3rd: add type-remark and classname</span></div><div class='line' id='LC118'>	<span class="k">if</span> <span class="nb">type</span> <span class="ow">is</span> <span class="ow">not</span> <span class="bp">None</span><span class="p">:</span></div><div class='line' id='LC119'>		<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;/** @remark &lt;b&gt;&quot;</span> <span class="o">+</span> <span class="nb">type</span> <span class="o">+</span> <span class="s">&quot;&lt;/b&gt; */&quot;</span><span class="p">)</span></div><div class='line' id='LC120'>	<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;/** @class &quot;</span> <span class="o">+</span> <span class="n">namespace</span><span class="o">+</span><span class="s">&quot;::&quot;</span><span class="o">+</span><span class="n">className</span> <span class="o">+</span> <span class="s">&quot; */&quot;</span><span class="p">)</span></div><div class='line' id='LC121'>	<span class="c"># 4th: dump remaining file </span></div><div class='line' id='LC122'>	<span class="n">showLines</span><span class="p">(</span><span class="n">txt</span><span class="p">)</span></div><div class='line' id='LC123'>	<span class="c"># 5th: close dummy class</span></div><div class='line' id='LC124'>	<span class="n">writeLine</span><span class="p">(</span><span class="s">&quot;}&quot;</span><span class="p">)</span></div><div class='line' id='LC125'><br/></div><div class='line' id='LC126'><span class="c">## @returns the complete file content as an array of lines</span></div><div class='line' id='LC127'><span class="k">def</span> <span class="nf">readFile</span><span class="p">(</span><span class="n">filename</span><span class="p">):</span></div><div class='line' id='LC128'>	<span class="n">f</span> <span class="o">=</span> <span class="nb">open</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC129'>	<span class="n">r</span> <span class="o">=</span> <span class="n">f</span><span class="o">.</span><span class="n">readlines</span><span class="p">()</span></div><div class='line' id='LC130'>	<span class="n">f</span><span class="o">.</span><span class="n">close</span><span class="p">()</span></div><div class='line' id='LC131'>	<span class="k">return</span> <span class="n">r</span></div><div class='line' id='LC132'><br/></div><div class='line' id='LC133'><span class="c">## dump all lines to stdout</span></div><div class='line' id='LC134'><span class="k">def</span> <span class="nf">showLines</span><span class="p">(</span><span class="n">r</span><span class="p">):</span></div><div class='line' id='LC135'>	<span class="k">for</span> <span class="n">s</span> <span class="ow">in</span> <span class="n">r</span><span class="p">:</span></div><div class='line' id='LC136'>		<span class="n">sys</span><span class="o">.</span><span class="n">stdout</span><span class="o">.</span><span class="n">write</span><span class="p">(</span><span class="n">s</span><span class="p">)</span></div><div class='line' id='LC137'><br/></div><div class='line' id='LC138'><span class="c">## main method - open a file and see what can be done</span></div><div class='line' id='LC139'><span class="k">def</span> <span class="nf">filter</span><span class="p">(</span><span class="n">filename</span><span class="p">):</span></div><div class='line' id='LC140'>	<span class="k">try</span><span class="p">:</span></div><div class='line' id='LC141'>		<span class="n">root</span><span class="p">,</span> <span class="n">ext</span> <span class="o">=</span> <span class="n">os</span><span class="o">.</span><span class="n">path</span><span class="o">.</span><span class="n">splitext</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC142'>		<span class="n">txt</span> <span class="o">=</span> <span class="n">readFile</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC143'>		<span class="k">if</span> <span class="p">(</span><span class="n">ext</span><span class="o">.</span><span class="n">lower</span><span class="p">()</span> <span class="o">==</span> <span class="s">&quot;.frag&quot;</span><span class="p">):</span></div><div class='line' id='LC144'>			<span class="n">parseShader</span><span class="p">(</span><span class="n">filename</span><span class="p">,</span> <span class="n">txt</span><span class="p">,</span> <span class="s">&quot;Fragment-Shader&quot;</span><span class="p">)</span></div><div class='line' id='LC145'>		<span class="k">elif</span> <span class="p">(</span><span class="n">ext</span><span class="o">.</span><span class="n">lower</span><span class="p">()</span> <span class="o">==</span> <span class="s">&quot;.vert&quot;</span><span class="p">):</span></div><div class='line' id='LC146'>			<span class="n">parseShader</span><span class="p">(</span><span class="n">filename</span><span class="p">,</span> <span class="n">txt</span><span class="p">,</span> <span class="s">&quot;Vertex-Shader&quot;</span><span class="p">)</span></div><div class='line' id='LC147'>		<span class="k">else</span><span class="p">:</span></div><div class='line' id='LC148'>			<span class="n">showLines</span><span class="p">(</span><span class="n">txt</span><span class="p">)</span></div><div class='line' id='LC149'>	<span class="k">except</span> <span class="ne">IOError</span><span class="p">,</span><span class="n">e</span><span class="p">:</span></div><div class='line' id='LC150'>		<span class="n">sys</span><span class="o">.</span><span class="n">stderr</span><span class="o">.</span><span class="n">write</span><span class="p">(</span><span class="n">e</span><span class="p">[</span><span class="mi">1</span><span class="p">]</span><span class="o">+</span><span class="s">&quot;</span><span class="se">\n</span><span class="s">&quot;</span><span class="p">)</span></div><div class='line' id='LC151'><br/></div><div class='line' id='LC152'><span class="k">if</span> <span class="nb">len</span><span class="p">(</span><span class="n">sys</span><span class="o">.</span><span class="n">argv</span><span class="p">)</span> <span class="o">!=</span> <span class="mi">2</span><span class="p">:</span></div><div class='line' id='LC153'>	<span class="k">print</span> <span class="s">&quot;usage: &quot;</span><span class="p">,</span> <span class="n">sys</span><span class="o">.</span><span class="n">argv</span><span class="p">[</span><span class="mi">0</span><span class="p">],</span> <span class="s">&quot; filename&quot;</span></div><div class='line' id='LC154'>	<span class="n">sys</span><span class="o">.</span><span class="n">exit</span><span class="p">(</span><span class="mi">1</span><span class="p">)</span></div><div class='line' id='LC155'><br/></div><div class='line' id='LC156'><span class="n">filename</span> <span class="o">=</span> <span class="n">sys</span><span class="o">.</span><span class="n">argv</span><span class="p">[</span><span class="mi">1</span><span class="p">]</span> </div><div class='line' id='LC157'><span class="nb">filter</span><span class="p">(</span><span class="n">filename</span><span class="p">)</span></div><div class='line' id='LC158'><span class="n">sys</span><span class="o">.</span><span class="n">exit</span><span class="p">(</span><span class="mi">0</span><span class="p">)</span></div></pre></div>
          </td>
        </tr>
      </table>
  </div>

          </div>
        </div>

        <a href="#jump-to-line" rel="facebox[.linejump]" data-hotkey="l" class="js-jump-to-line" style="display:none">Jump to Line</a>
        <div id="jump-to-line" style="display:none">
          <form accept-charset="UTF-8" class="js-jump-to-line-form">
            <input class="linejump-input js-jump-to-line-field" type="text" placeholder="Jump to line&hellip;">
            <button type="submit" class="button">Go</button>
          </form>
        </div>

</div>

<div id="js-frame-loading-template" class="frame frame-loading large-loading-area" style="display:none;">
  <img class="js-frame-loading-spinner" src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-128.gif" height="64" width="64">
</div>


          </div>
        </div>

      </div><!-- /.repo-container -->
      <div class="modal-backdrop"></div>
    </div>
  </div><!-- /.site -->


    </div><!-- /.wrapper -->

      <div class="container">
  <div class="site-footer">
    <ul class="site-footer-links right">
      <li><a href="https://status.github.com/">Status</a></li>
      <li><a href="http://developer.github.com">Developer</a></li>
      <li><a href="http://training.github.com">Training</a></li>
      <li><a href="http://shop.github.com">Shop</a></li>
      <li><a href="/blog">Blog</a></li>
      <li><a href="/about">About</a></li>
    </ul>

    <a href="/">
      <span class="mega-octicon octicon-mark-github"></span>
    </a>

    <ul class="site-footer-links">
      <li>&copy; 2013 <span title="0.08861s from fe2.rs.github.com">GitHub</span>, Inc.</li>
        <li><a href="/site/terms">Terms</a></li>
        <li><a href="/site/privacy">Privacy</a></li>
        <li><a href="/security">Security</a></li>
        <li><a href="/contact">Contact</a></li>
    </ul>
  </div><!-- /.site-footer -->
</div><!-- /.container -->


    <div class="fullscreen-overlay js-fullscreen-overlay" id="fullscreen_overlay">
  <div class="fullscreen-container js-fullscreen-container">
    <div class="textarea-wrap">
      <textarea name="fullscreen-contents" id="fullscreen-contents" class="js-fullscreen-contents" placeholder="" data-suggester="fullscreen_suggester"></textarea>
          <div class="suggester-container">
              <div class="suggester fullscreen-suggester js-navigation-container" id="fullscreen_suggester"
                 data-url="/numb3r23/glslAdditions/suggestions/commit">
              </div>
          </div>
    </div>
  </div>
  <div class="fullscreen-sidebar">
    <a href="#" class="exit-fullscreen js-exit-fullscreen tooltipped leftwards" title="Exit Zen Mode">
      <span class="mega-octicon octicon-screen-normal"></span>
    </a>
    <a href="#" class="theme-switcher js-theme-switcher tooltipped leftwards"
      title="Switch themes">
      <span class="octicon octicon-color-mode"></span>
    </a>
  </div>
</div>



    <div id="ajax-error-message" class="flash flash-error">
      <span class="octicon octicon-alert"></span>
      <a href="#" class="octicon octicon-remove-close close ajax-error-dismiss"></a>
      Something went wrong with that request. Please try again.
    </div>

    
    <span id='server_response_time' data-time='0.08902' data-host='fe2'></span>
    
  </body>
</html>

