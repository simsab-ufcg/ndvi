logger <- function(description){
    t <- Sys.time()
    typeof(t)
    cat(t, description,'\n')
}