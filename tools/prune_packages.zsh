# script to prune old github packages
# requires https://cli.github.com
# requires https://stedolan.github.io/jq/
# brew install gh jq

# Need to login to github first using and request rights to delete packages
# gh auto login -s read:packages,write:packages,delete:packages

# We prune all packages older than prune_older
prune_older="2023-09-01"

for page in 1 2 3 4 
do
    for package in `gh api -H "X-GitHub-Api-Version: 2022-11-28" -H "Accept: application/vnd.github+json" "/orgs/inviwo/packages?package_type=nuget&per_page=100&page=$page" | jq -r '.[].name'`
    do
        vers=`gh api -H "X-GitHub-Api-Version: 2022-11-28" -H "Accept: application/vnd.github+json" "/orgs/inviwo/packages/nuget/$package/versions?per_page=100"`

        len=`echo $vers | jq '. | length'`
        echo "Package $package: versions: $len"

        # We want to keep at least 1 so start a 1 instead of 0
        for i in `seq 1 $((len-1))`
        do
            id=`echo $vers | jq ".[$i].id"`
            created_at=`echo $vers | jq -r ".[$i].created_at"`

            if [[ "$created_at" < "$prune_older" ]]
            then
                echo "$i $id -> $created_at Delete" 

                gh api --method DELETE -H "X-GitHub-Api-Version: 2022-11-28" -H "Accept: application/vnd.github+json" "/orgs/inviwo/packages/nuget/$package/versions/$id" 

            else
                echo "$i $id -> $created_at"
            fi
        done
    done
done 
