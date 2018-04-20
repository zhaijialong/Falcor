import os	
import sys
import argparse
import shutil 
import subprocess
import time
import stat
import xml.etree.ElementTree as ET 
import re

#for debugging prop sheet xml
def printTree(node, depth = 0, childIndex = 0):
	for i in range(0, depth):
		print('\t', end='')
	print(childIndex, node.tag, node.attrib, node.text)
	for i, child in enumerate(node):
		printTree(child, depth + 1, i)

def getNamespace(node):
	m = re.match('\{(.*)\}', node.tag)
	return m.group(1) if m.group(1) is not None else ''
	
def patchPropertySheet(propSheetPath):
	tree = ET.parse(propSheetPath)
	root = tree.getroot()
	defaultNs = getNamespace(root) 
	ET.register_namespace('', defaultNs)
	nsDict = {'ns' : defaultNs}
	#add prebuild step
	itemDefGroup = tree.find('./ns:ItemDefinitionGroup', nsDict)
	preBuild = ET.SubElement(itemDefGroup, '{' + nsDict['ns'] + '}PreBuildEvent')
	pbCommand = ET.SubElement(preBuild, '{' + nsDict['ns'] + '}Command')
	pbCommand.text = 'call $(FALCOR_CORE_DIRECTORY)\BuildScripts\prebuild.bat $(FALCOR_CORE_DIRECTORY) $(SolutionDir) $(ProjectDir) $(PlatformName) $(PlatformShortName) $(Configuration) $(OutDir)'
	tree.write(propSheetPath)
	
def buildLibrary(args):
	if subprocess.call(args) != 0:
		print('Error building config ' + args[3])
		sys.exit()
		
def buildAndCopyLibraries(backends):
	buildScript = os.path.join(args.falcorDirectory, 'Tests', 'BuildSolution.bat')
	solutionPath = os.path.join(args.falcorDirectory, 'Falcor.sln')
	resultLibDir = os.path.join(resultFwDir, 'Lib')
	os.mkdir(resultLibDir)
	
	for backend in backends: 
		#Build
		debugConfig = 'debug' + backend 
		releaseConfig = 'release' + backend 
		buildArgs = [buildScript, 'rebuild', solutionPath, debugConfig, 'Falcor']
		buildLibrary(buildArgs)
		#sometimes files are still in use and build fails if tries second build immediately 
		time.sleep(5)
		buildArgs[3] = releaseConfig
		buildLibrary(buildArgs)
		
		#Copy
		libParentDir = os.path.join(args.falcorDirectory, 'Bin', 'Int', 'x64')
		debugLibPath = os.path.join(libParentDir, debugConfig, 'Lib', 'Falcor.lib')
		releaseLibPath = os.path.join(libParentDir, releaseConfig, 'Lib', 'Falcor.lib')
		resultDebugLibDir = os.path.join(resultLibDir, debugConfig)
		os.mkdir(resultDebugLibDir)
		resultReleaseLibDir = os.path.join(resultLibDir, releaseConfig)
		os.mkdir(resultReleaseLibDir)
		shutil.copy(debugLibPath, os.path.join(resultDebugLibDir, 'Falcor.lib'))
		shutil.copy(releaseLibPath, os.path.join(resultReleaseLibDir, 'Falcor.lib'))
	
#setup args
parser = argparse.ArgumentParser()
parser.add_argument('-fd', '--falcorDirectory', action='store', help='Specify the location of falcor\'s top level directory.')
parser.add_argument('-rd', '--resultDirectory', action='store', help='[Optional] Specify the location of the result directory')
parser.add_argument('-d3d', '--d3d12', action='store_true', help='[Optional]Include D3D12 backend (All backends if none are specified )')
parser.add_argument('-dxr', '--dxr', action='store_true', help='[Optional]Include DXR backend (All backends if none are specified )')
parser.add_argument('-vk', '--vulkan', action='store_true', help='[Optional]Include Vulkan backend (All backedns if none are specified )')
args = parser.parse_args()

#check given falcor dir and save relevant paths   
if not args.falcorDirectory:
	print('Error! Please specify falcor\'s directory with -fd or --falcorDirectory')
	sys.exit()
else:
	falcorFwDir = os.path.join(args.falcorDirectory, 'Framework')
	falcorSourceDir = os.path.join(args.falcorDirectory, 'Framework', 'Source')
	#check a few folders and for the presence of the sln to make sure the dir provided actually is a falcor repo
	if not os.path.isdir(args.falcorDirectory) or not os.path.isdir(falcorFwDir) or not os.path.isdir(falcorSourceDir) or not os.path.isfile(os.path.join(args.falcorDirectory, 'Falcor.sln')):
		print('Cant find expected directories for files in supplied falcor dir : ' + args.falcorDirectory)
		sys.exit()
	else:
		args.falcorDirectory = os.path.abspath(args.falcorDirectory)

#check for passed-in result directory 
if not args.resultDirectory:
	print('No result directory specified. Using default (working directory)')
	resultParentDir = os.getcwd()
else:
	resultParentDir = args.resultDirectory
	
#check what backends are desired 
if not args.d3d12 and not args.dxr and not args.vulkan:
	print('No backends specified. Including all backends (vk, d3d, and dxr)')
	args.d3d12 = True 
	args.dxr = True 
	args.vulkan = True
	
#make result dir, delete if already exists
resultDir = os.path.join(resultParentDir, 'FalcorPackage')
if os.path.isdir(resultDir):
	try: 
		shutil.rmtree(resultDir)
	except: 
		#todo figure out why getting access is denied so this doesn't happen
		print('Error trying to remove existing ' + resultDir)
		print('Please manually delete it and try again')
os.mkdir(resultDir)

#Top level dir
#dependencies.xml
dependsFile = 'dependencies.xml'
dependsSrcPath = os.path.join(args.falcorDirectory, dependsFile)
dependsDstPath = os.path.join(resultDir, dependsFile)
shutil.copy(dependsSrcPath, dependsDstPath)
updateDependsPrefix = 'update_dependencies'
#update_dependencies
updateDependsBat = updateDependsPrefix + '.bat'
updateDependsSh = updateDependsPrefix + '.sh'
updateDependsBatSrcPath = os.path.join(args.falcorDirectory, updateDependsBat)
updateDependsShSrcPath = os.path.join(args.falcorDirectory, updateDependsSh)
updateDependsBatDstPath = os.path.join(resultDir, updateDependsBat)
updateDependsShDstPath = os.path.join(resultDir, updateDependsSh)
shutil.copy(updateDependsBatSrcPath, updateDependsBatDstPath)
shutil.copy(updateDependsShSrcPath, updateDependsShDstPath)
#packman dir
packmanDirSrcPath = os.path.join(args.falcorDirectory, 'packman')
packmanDirDstPath = os.path.join(resultDir, 'packman')
shutil.copytree(packmanDirSrcPath, packmanDirDstPath)

#copy headers
print('Copying Headers')
resultFwDir = os.path.join(resultDir, 'Framework')
os.mkdir(resultFwDir)
resultSourceDir = os.path.join(resultFwDir, 'Source')
os.mkdir(resultSourceDir)
for subdir, dirs, files in os.walk(falcorSourceDir):
	for file in files:
		if not (file.endswith('.cpp') or file.endswith('.user')):
			fullSrcPath = os.path.join(subdir, file)
			fullDstPath = os.path.join(resultSourceDir, file)
			shutil.copy(fullSrcPath, fullDstPath)
	for dir in dirs:
		fullSrcPath = os.path.join(subdir, dir)
		fullDstPath = os.path.join(resultSourceDir, dir)
		shutil.copytree(fullSrcPath, fullDstPath, ignore=shutil.ignore_patterns('*.cpp'))
	#doing a directory copy so I don't actually need to recurse, just do first loop
	#could probably just do os.listdir but this differentiates between files and dirs automatically
	break
		
#Build and Copy Libraries 
print('Building and Copying Libraries')
backendList = []
if args.vulkan:
	backendList.append('vk')
if args.d3d12:
	backendList.append('d3d12')
if args.dxr:
	backendList.append('dxr')
buildAndCopyLibraries(backendList)

#Copy Build Scripts 
print('Copying Build Scripts')
srcFwDir = os.path.join(resultDir, 'Framework')
buildScriptsSrcDir = os.path.join(args.falcorDirectory, 'Framework', 'BuildScripts')
print(buildScriptsSrcDir)
buildScriptsDstDir = os.path.join(resultFwDir, 'BuildScripts')
print(resultFwDir)
print(buildScriptsDstDir)
shutil.copytree(buildScriptsSrcDir, buildScriptsDstDir, ignore=shutil.ignore_patterns('MakePackage'))
#move the readme out of build scripts into the main dir 
readmeSrcPath = os.path.join(buildScriptsSrcDir, 'MakePackage', 'README.txt')
shutil.copy(readmeSrcPath, os.path.join(resultDir, 'README.txt'))

#patch property sheet 
propSheetPath = os.path.join(resultSourceDir, 'Falcor.props')
patchPropertySheet(propSheetPath)