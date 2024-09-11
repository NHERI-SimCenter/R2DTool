
echo In folder $PWD
   
# Clone the examples
git clone --branch master --depth 1 https://github.com/NHERI-SimCenter/R2DExamples.git

# Add the current dir in the example file
sed -i "s|{Current_Dir}|$(pwd)|g" $PWD/R2DExamples/E1BasicHAZUS/input.json

# Create the working directoy
mkdir tmp.SimCenter

# Copy over the input data
cp -R $PWD/R2DExamples/E1BasicHAZUS/input_data .

# Run the example in the backend
python $PWD/SimCenterBackendApplications/applications/Workflow/rWHALE.py ./R2DExamples/E1BasicHAZUS/input.json --registry $PWD/SimCenterBackendApplications/applications/Workflow/WorkflowApplications.json --referenceDir $PWD/input_data -w $PWD/tmp.SimCenter/Results

