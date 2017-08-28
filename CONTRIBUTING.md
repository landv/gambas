# How To Contribute

This guide provides a step-by-step tutorial on how to contribute to the Gambas source code and translations.

It will cover the sources' organization, some contributing guidelines and how to use Git and GitLab in order to submit your contribution.

## Using Git and GitLab

The Gambas source code is managed by a Git repository, hosted on [GitLab.com](https://gitlab.com/gambas/gambas).

To handle new contributions, we use the [Project forking workflow](https://docs.gitlab.com/ce/workflow/forking_workflow.html) : 
since contributors do not have the permission to write to the Gambas source code directly, you will have to create a separate repository containing your changes, and then create a merge request asking the Gambas developers to merge your changes into the main repository.

While this might sound complex to new contributors, this document is made to guide through this process, step by step.

If you are having trouble with the steps mentioned here, or if you have any question regarding a contribution, you can [ask on the mailing-list](http://gambaswiki.org/wiki/doc/forum).

### Creating a GitLab account

First, you will need a [GitLab account](https://gitlab.com/users/sign_in) in order to submit any changes.

We also recommend you [use SSH to work with Git repositories](https://docs.gitlab.com/ce/ssh/README.html), instead of HTTPS.
Not only you won't have to enter your GitLab username and password every time you want to interact with the repository, but it is also more secure as your password is never sent through the network.

You can also [use GPG to sign your commits](https://docs.gitlab.com/ee/user/project/gpg_signed_commits/index.html), although it is not required.

### Forking the Gambas Repository

Now that your GitLab account is set up, we can now [fork the Gambas repository](https://docs.gitlab.com/ce/gitlab-basics/fork-project.html).
This will create a copy of the Gambas repository, but it will belong to you, so you can make any change you want.

To do this, just go over to the [Gambas project page](https://gitlab.com/gambas/gambas), and click the "Fork" button.

You will then be asked where to put the forked repository. Once complete, the new repository will appear under your account.

You can then clone the repository to your local machine, using the following command (replace `<yourusername>` with your GitLab username):

    git clone git@gitlab.com:<yourusername>/gambas.git

### Making changes to your repository

Once the cloning is complete, you can make changes to your local copy, which will then have to be commited and pushed.

First, you can check which files you changed with the `git status`. It's always good to check before commiting !
You can also view the full diff with the `git diff` command.

Once everything is done, you will have to select which files you want to commit next using the [git add](https://git-scm.com/docs/git-add) command.

You can either select specific files or directories using `git add file1.c file2.c main/gbx`, or just select everything using `git add -A`.

You can then make your commit using the `git commit` command. 
This command will open an editor to let you write your commit message.

For some guidelines on how to write commit messages, see the Writing commit messages section.

This command will start the default editor (usually `vi`), but you can change this by setting the `EDITOR` environment variable to the command starting your favorite editor.

Now that the commit is done, you can push it to your GitLab repository using the `git push` command.

### Creating the merge request

With your changes now pushed to the GitLab repository, the final step is to create a [Merge Request](https://docs.gitlab.com/ee/user/project/merge_requests/index.html), 
kindly asking the Gambas developers to merge your changes to the main Gambas repository.

Since this process is entirerly made through GitLab, [use the following instructions to create your merge requests](https://docs.gitlab.com/ee/gitlab-basics/add-merge-request.html).

Its is probable that your changes won't be accepted right away, and you will be asked by the Gambas maintainers to make some changes.

In this case, you can make your changes, commit them and then push them. 
The Merge request on GitLab will be automatically updated, you won't have to recreate it.

### Keeping your repository up to date

During the time you make changes to your version of the Gambas source code, or while your request is being reviewed, it is likely that the Gambas repository will receive some updates.

Since a forked repository is basically a clone, the official version and your version are completely separate, it will not receive newer commits automatically.

However, you can setup your local repository to connect to both repositories, so you can pull changes from the official Gambas repository, merge them with your changes locally, and then push them to your forked repository.

First, we will setup your local Git repository by adding the original Gambas repository as a second remote :

    git remote add upstream https://gitlab.com/gambas/gambas.git

We now have added a new remote named `upstream` to the local repository, pointing to the original Gambas repository.
(You can list all the remotes with the `git remote -v` command.)

This means Git can now pull changes from the original Gambas repository using the following command :

    git pull upstream master

This command will take the changes from the `master` branch of the `upstream` remote. 

If you made commits to your version of the repository, it will merge them with the new changes, creating a new merge commit.

When the merge is complete, you can simply use `git push` to push all these changes to your version of the repository. If you have any Merge Request pending, they will get updated automatically.


## Writing commit messages

In order to automatically generate changelogs for each release, commits in the Gambas repository have to follow a very specific format.

Here is an example :

    This commit contains things, adds stuff and has lots of fluff.
    (but this won't go in the changelog)
    
    [GB.QT4]
    * NEW: Added things to the component.
    * BUG: Fixed a bug in the Foo function.
    * NEW: Added this very long modification...
      ...and it takes more than one Line To Write it.
    
    This won't go into the changelog either.
    
    [COMPILER]
    * BUG: What an awful bug!
    * OPT: Make things go faster.
    
    [GB.GTK3]
    * NEW: The component is now complete!

As per the Git commit message convention, the first line of each commit is a short description of what it contains. 
This line does not end up in the changelog, but it appears in git logs, as well as in the GitLab interface.

Then, the commit message consists of the following parts : 

* A slot, between square brackets (e.g. `[GB.QT4]`)
* One or more modifications, each prefixed with a tag, which is either `* NEW: `, `* BUG: `, or `* OPT: `, with a space at the end.

The slot's name is the one of the component modified (in uppercase), or one of these if the changes do not affect a component :
* `[INTERPRETER]` for changes in the interpreter (gbx3).
* `[COMPILER]` for changes in the compiler (gbc3).
* `[ARCHIVER]` for changes in the archiver (gba3).
* `[INFORMER]` for changes in the informer (gbi3).
* `[DEVELOPMENT ENVIRONMENT]` for changes in the IDE (gambas3).
* `[CONFIGURATION]` for changes in the automake/autoconf configuration process
* `[WIKI CGI SCRIPT]` for changes in the wiki CGI script.
* `[WEB SITE MAKER]` for changes in the Gambas web site generator.
* `[EXAMPLES]` for changes in any example.

The tag's name is one of the following:

* `NEW` is for new features or translations, updates or other improvements;
* `BUG` is for bug fixes and other corrections
* `OPT` is for optimizations

Things without an impact for the user (such as refactorings or code cleanups) should not end up in the Changelog.

All lines without a tag will not appear in the changelog, but if you want a modification to span across multiple lines, you will have to prefix it with two spaces.
