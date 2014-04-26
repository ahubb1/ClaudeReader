 #!/bin/bash
#A simple git commit script
echo --------Commit---------
echo Enter a comment line:
read comment
git commit -a -m "$comment"
git push origin master
exit