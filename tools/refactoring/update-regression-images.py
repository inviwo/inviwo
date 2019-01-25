import json
import argparse
import requests


def main():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument('-report', default='report.json',  help='location of report.json file')
    parser.add_argument('-regressiondir', help='regression directory')
    args = parser.parse_args()

    jobbase = "http://jenkins.inviwo.org:8080/job/inviwo/job/feature%252Fworkspaces2"
    dirbase = "/Users/jenkins/jenkins/workspace/inviwo_feature_workspaces2/inviwo/"

    local = "C:/Users/petst55/Work/Inviwo/Inviwo-dev/"

    request = requests.get(jobbase + '/Regression/report.json')

    report = request.json()
    for testName, testResult in report.items():
        print(testName)
        if 'failures' in testResult.keys():
            if 'image_tests' in testResult['failures'].keys():
                for image, message in testResult['failures']['image_tests'].items():
                    imgpath = jobbase + "/Regression/" + testResult['outputdir'].split('/regress/')[1] + "/imgtest/" + image 
                    destpath = local + testResult['path'].split(dirbase)[1] + "/" + image
                    print("   " + imgpath)
                    print("   " + destpath)
                    img = requests.get(imgpath)
                    with open(destpath, "wb") as f:
                        f.write(img.content)


if __name__ == '__main__':
    main()



#'http://jenkins.inviwo.org:8080/job/inviwo/job/feature%252Fworkspaces2/Regression/userinterfacegl/cropvolume/2019-01-25T11_53_35.508429/imgtest/CanvasGL.png'
#              /Users/jenkins/jenkins/workspace/inviwo_feature_workspaces2/regress/vectorfieldvisualizationgl/lic_rk4/2019-01-25T12_07_15.211025
