
# Programming assignment, writeup

### importing, peprocessing data 

  + Get a local copy of the training and test csv file. Assume they are in "/Users/kgeorge/Downloads/pml-training.csv" and "/Users/kgeorge/Downloads/pml-testing.csv"  
<pre>
	// read training data into data_training_raw
 	data_training_raw = read.csv("/Users/kgeorge/Downloads/pml-training.csv", header=TRUE, na.strings=c("#DIV/0!"))

	// get another coopy of the training data
 	data_training_raw_copy = read.csv("/Users/kgeorge/Downloads/pml-training.csv", header=TRUE, na.strings=c("#DIV/0!"))
 	
	// read testing data into data_testing_raw
	data_testing_raw = read.csv("/Users/kgeorge/Downloads/pml-testing.csv", header=TRUE, na.strings=c("#DIV/0!"))
</pre>

   + The first row of the csv file contains column names, so <code>header=TRUE</code> in the above code. Also, there are entries whose values are #DIV/0! in some cells, (row & column), Such entries should be treated as "NA". Hence the option <code>na.strings=c("#DIV/0!")</code>

   + Let us first work on the <code>data_training_raw</code>. Let us select only the numeric columns from <code>data_training_raw</code> into <code>data_training_relevant</code>
   <pre>
	numeric_columns <- sapply(data_training_raw, is.numeric)
	data_training_relevant <- data_training_raw[, numeric_columns]
   </pre>

  + Some of the non-numeric columns that we filtered out in the above code are still relevant, so they need be put back in

   <pre>
	data_training_relevant$user_name = data_training_raw$user_name
	data_training_relevant$new_window = data_training_raw$new_window
	data_training_relevant$classe = data_training_raw$classe
   </pre>

  + Next let us preprocess the testing data also along this line. We should select all the columns that are present in the <code>data_training_relevant</code> to be present in the testing data.a Please note that the output column <code>classe</code> is absent in the testing data. So, in effect we will be making sure the tssting data contains exactly only those columns in <code>data_training_relevant</code> except for <code>classe</code>
 <pre>
	column_names_from_training_data_minus_classe = colnames(data_training_relevant)[1:(dim(data_training_relevant)[2]-1)]
        data_testing_relevant = subset(data_testing_raw, select=c(column_names_from_training_data_minus_classe))
	// add a classe column to the data_testing_relevant, 
	// and fillit with some dummy values
	data_testing_relevant$classe = data_taining_relevant$classe[1:dim(data_testing_relevant)[1]]
 </pre>
  + Let us try to pre-process the training and test data by using either pca or just simple centering and scaling. We choose to use a simple centering and scaling (mean normalization), since our attempt to get pca met with errors. 
  <pre>
	data_training_numeric_only = subset(data_training_relevant, select=-c(user_name, new_window, classe))
	preproc <- preProcess(data_training_numeric_only, method=c("center", "scale"),  na.remove=TRUE)
	data_training_mean_normalized <- predict(preproc, data_training_numeric_only)
	
	data_testing_numeric_only <- subset(data_testing_relevant, select=-c(user_name, new_window, classe))
	data_testing_mean_normalized <- predict(preproc, data_testing_numeric_only)

	data_training_mean_normalized$user_name = data_training_relevant$user_name
	data_training_mean_normalized$new_window = data_training_relevant$new_window
	data_training_mean_normaized$classe = data_training_relevant$classe
	
	data_testing_mean_normalized$user_name = data_testing_relevant$user_name
	data_testing_mean_normalized$new_window = data_testing_relevant$new_window
	data_testing_mean_normaized$classe = data_testing_relevant$classe
	
  </pre> 

  + Let us use a k-fold cross-validation with k=10. Cross validation can be easily achieved by means of <code>trainControl</code> in caret package. Let us try with a few methods, a gradient boostig method and a random firest method 

   <pre>
      trControl = trainControl(method = "cv", number = 10)
      modelFitGbm <- train(classe ~., method="gbm", data=data_training_mean_normalized, verbose=FALSE, trControl = trControl)
      modelFitRf <- train(classe ~., method="rf", data=data_training_mean_normalized, verbose=FALSE, trControl = trControl) 

   </pre>

  + We expect both of yhese methods to tune the model with cross  validation and report us back with an accuracy. If we get an accuracy of 0.99, then we should be in good shape.
   





